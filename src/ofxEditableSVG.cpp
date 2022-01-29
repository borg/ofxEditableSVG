#include "ofxEditableSVG.h"

#include "Poco/MD5Engine.h"
#include "Poco/SHA1Engine.h"
#include "Poco/DigestStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/Base64Decoder.h"
#include "Poco/Base64Encoder.h"

using Poco::DigestEngine;
using Poco::MD5Engine;
using Poco::SHA1Engine;
using Poco::Base64Encoder;
using Poco::Base64Decoder;
using Poco::DigestOutputStream;
using Poco::StreamCopier;

using namespace std;
using namespace Poco::XML;


ofxEditableSVG::ofxEditableSVG(){
    ofPtr<svgNode> rootnode(new svgNode());
    rootnode->type = SVG_TAG_TYPE_DOCUMENT;
    info.rootnode = rootnode;
    x=0;
    y=0;
    
    //set default values
    setViewbox(0,0,100,100);
    //this is important on iOS...maybe higher on OSX
    setComplexityThreshold(5000);
    
};
ofxEditableSVG::~ofxEditableSVG(){
	paths.clear();
    info.rootnode.reset();
    for(int i = 0; i < info.flatlist.size(); i++){
        info.flatlist[i].reset();
    }
    info.flatlist.clear();
    //xmlFree(svgDocument);
    
}


int ofxEditableSVG::getNumPath(){
    return paths.size();
}
ofPath & ofxEditableSVG::getPathAt(int n){
    return *paths[n];
}

void ofxEditableSVG::load(string path){
	path = ofToDataPath(path);
    
	if(path.compare("") == 0){
		stringstream ss;
		ss << path << " does not exist " << endl;
		ofLog(OF_LOG_ERROR, ss.str());
		return;
	}
    
	ofBuffer buffer = ofBufferFromFile(path);
	//size_t size = buffer.size();
    
    parseXML(buffer.getText());
    
    
    
    
    
    // ofLog()<<"paths size "<<info.rootnode->type<<" getNumPath "<<getNumPath()<<" " <<paths.size()<<endl;
    
}


void ofxEditableSVG::parseXML(string xml){
    
    
    if( info.rootnode){
        info.rootnode.reset();
    }
    
    info.flatlist.clear();
    
    ofPtr<svgNode> rootnode(new svgNode());
    rootnode->type = SVG_TAG_TYPE_DOCUMENT;
    info.rootnode = rootnode;
    x=0;
    y=0;
    if(paths.size()>0){
        paths.clear();
    }
    
    
    struct svgtiny_diagram * diagram = svgtiny_create();
    ofLog()<<"parseXML "<<xml;
	svgtiny_code code = svgtiny_parse(info,diagram, xml.c_str(), 0, 0);
    //svgtiny_parse(diagram, buffer.getText().c_str(), size, path.c_str(), 0, 0);
    
    
    
    
    if(code != svgtiny_OK){
		fprintf(stderr, "svgtiny_parse failed: ");
		switch(code){
            case svgtiny_OUT_OF_MEMORY:
                fprintf(stderr, "svgtiny_OUT_OF_MEMORY");
                break;
                
            case svgtiny_LIBXML_ERROR:
                fprintf(stderr, "svgtiny_LIBXML_ERROR");
                break;
                
            case svgtiny_NOT_SVG:
                fprintf(stderr, "svgtiny_NOT_SVG");
                break;
                
            case svgtiny_SVG_ERROR:
                fprintf(stderr, "svgtiny_SVG_ERROR: line %i: %s",
                        diagram->error_line,
                        diagram->error_message);
                break;
                
            default:
                fprintf(stderr, "unknown svgtiny_code %i", code);
                break;
		}
		fprintf(stderr, "\n");
	}
    
    ofLog()<<"diagram "<<diagram;
    
	setupDiagram(diagram);
    
	svgtiny_free(diagram);
    
    //Check if path to complex. Simplify if so.
    
    for(int i = 0; i < paths.size(); i++){
        if(paths[i]->getCommands().size()>complexityThreshold){
            paths[i]->simplify(.3);
            ofLogWarning()<<"ofxEditableSVG too complex for tesselator, it was simplified. Cmd size: "<<paths[i]->getCommands().size()<<endl;
        }
    }
}

void ofxEditableSVG::draw(){
    
    ofPushMatrix();
    ofTranslate(x,y);
    
	for(int i = 0; i < paths.size(); i++){
        paths[i]->draw();
 	}
    ofPopMatrix();
}


void ofxEditableSVG::setupDiagram(struct svgtiny_diagram * diagram){
    
	width = diagram->width;
	height = diagram->height;
    
	for(int i = 0; i < diagram->shape_count; i++){
		if(diagram->shape[i].path){
			setupShape(&diagram->shape[i]);
		}
		else if(diagram->shape[i].text){
			printf("text: not implemented yet\n");
		}
	}
}

void ofxEditableSVG::setupShape(struct svgtiny_shape * shape){
	float * p = shape->path;
    
	ofPath * path = new ofPath();
	paths.push_back(ofPathRef(path));
    
	path->setFilled(false);
    
	if(shape->fill != svgtiny_TRANSPARENT){
		path->setFilled(true);
		path->setFillHexColor(shape->fill);
	}
    
	if(shape->stroke != svgtiny_TRANSPARENT){
		path->setStrokeWidth(shape->stroke_width);
		path->setStrokeHexColor(shape->stroke);
	}
    
	for(int i = 0; i < shape->path_length;){
		if(p[i] == svgtiny_PATH_MOVE){
			path->moveTo(p[i + 1], p[i + 2]);
			i += 3;
		}
		else if(p[i] == svgtiny_PATH_CLOSE){
			path->close();
            
			i += 1;
		}
		else if(p[i] == svgtiny_PATH_LINE){
			path->lineTo(p[i + 1], p[i + 2]);
			i += 3;
		}
		else if(p[i] == svgtiny_PATH_BEZIER){
			path->bezierTo(p[i + 1], p[i + 2],
						   p[i + 3], p[i + 4],
						   p[i + 5], p[i + 6]);
			i += 7;
		}
		else{
			//cout << "error\n" << endl;
			ofLogError() << "SVG parse error";
			i += 1;
		}
	}
}





/*
 These affect the whole document
 */
void ofxEditableSVG::setFilled(bool t){
    
    for(int i=0;i<paths.size();i++){
        paths[i]->setFilled(t);
        if(t){
            setSVGattribute(info.flatlist[i],"fill_opacity","1.0");
        }else{
            setSVGattribute(info.flatlist[i],"fill_opacity","0.0");
        }
    }
    isFilled = t;
    // ofLog()<<"setFilled "<<t<<"  " <<paths.size()<<endl;
    
};


void ofxEditableSVG::setFilled(bool t,int path){
    if(path>info.flatlist.size()-1){
        ofLog()<<"WARNING: Tried to modify ofxEditableSVG path out of index - "<< path<<" of info.flatlist.size() "<<info.flatlist.size()<<endl;
        return;
    }
    paths[path]->setFilled(t);
    
    if(t){
        setSVGattribute(info.flatlist[path],"fill_opacity","1.0");
    }else{
        setSVGattribute(info.flatlist[path],"fill_opacity","0.0");
    }
    
    //ofLog()<<"setFilled path"<<t<<"  " <<path<<endl;
    
};



void ofxEditableSVG::setFillColor(ofColor col){
    //ofLog()<<"col.r "<<ofToHex(col.r)<<endl;
    for(int i=0;i<paths.size();i++){
        //col.set(255,255,255);
        paths[i]->setFillHexColor(col.getHex());
        //borg...update DOM ...
        setSVGattribute(info.flatlist[i],"fill",hexToWeb(col));
        setSVGattribute(info.flatlist[i],"fill_opacity",ofToString((float) col.a/255.0));
        
        //info.flatlist[i]->path.fill = hexToWeb(col);
        //info.flatlist[i]->path.fill_opacity = ofToString((float) col.a/255.0);
    }
    fill = col;
    // ofLog()<<"setFillColor"<<endl;
};


void ofxEditableSVG::setFillColor(ofColor col,int path){
    if(path>info.flatlist.size()-1){
        ofLog()<<"WARNING: Tried to modify ofxEditableSVG path out of index - "<< path<<" of info.flatlist.size() "<<info.flatlist.size()<<endl;
        return;
    }
    paths[path]->setFillHexColor(col.getHex());
    
    
    setSVGattribute(info.flatlist[path],"fill",ofToString(hexToWeb(col)));
    setSVGattribute(info.flatlist[path],"fill_opacity",ofToString((float) col.a/255.0));
    
    
    //  ofLog()<<"setFillColor path"<<"  " <<path<<" "<<ofToString(hexToWeb(col))<<" info.flatlist.size()  "<<info.flatlist.size()<< " paths.size() "<<paths.size()<<endl;
    
    
    //info.flatlist[path]->path.fill = hexToWeb(col);
    // info.flatlist[path]->path.fill_opacity = ofToString((float) col.a/255.0);
    
};




void ofxEditableSVG::setStrokeWidth(float f){
    // ofLog()<<"setStrokeWidth info.flatlist.size "<< info.flatlist.size()<<" paths.size() "<<paths.size()<<endl;
    
    
    for(int i=0;i<paths.size();i++){
        paths[i]->setStrokeWidth(f);
        //borg...update DOM...make sure you cast correctly as flatlist is using ofPtr...and xcode doesn't tell you float is not string
        
        //info.flatlist[i]->path.stroke_width = ofToString(f);
        setSVGattribute(info.flatlist[i],"stroke_width",ofToString(f));
        
        
    }
    
    strokeWidth = f;
};
void ofxEditableSVG::setStrokeColor(ofColor col){
    for(int i=0;i<paths.size();i++){
        paths[i]->setStrokeColor(col);
        //borg...update DOM
        setSVGattribute(info.flatlist[i],"stroke",hexToWeb(col));
        setSVGattribute(info.flatlist[i],"stroke_opacity",ofToString((float) col.a/255.0));
        //info.flatlist[i]->path.stroke = ofToString(hexToWeb(col));
        //info.flatlist[i]->path.stroke_opacity = ofToString((float) col.a/255.0);
    }
    stroke = col;
};



bool ofxEditableSVG::getFilled(){
    return isFilled;
};

ofColor ofxEditableSVG::getFillColor(){
    return fill;
};

float ofxEditableSVG::getStrokeWidth(){
    return strokeWidth;
};

ofColor ofxEditableSVG::getStrokeColor(){
    return stroke;
};



/*
 I wish there was a C++ equivalent of the AS3 this["attr"] syntax.
 Clever ways of mapping this welcomed.
 */


void ofxEditableSVG::setSVGattribute(ofPtr<svgNode>  node,string attribute, string value){
    switch (node->type){
        case SVG_TAG_TYPE_CIRCLE:
            if(attribute == "cx"){
                node->circle.cx = value;
            }
            if(attribute == "cy"){
                node->circle.cy = value;
            }
            if(attribute == "fill"){
                node->circle.fill = value;
            }
            if(attribute == "stroke_width"){
                node->circle.stroke_width = value;
            }
            if(attribute == "stroke"){
                node->circle.stroke = value;
            }
            if(attribute == "r"){
                node->circle.r = value;
            }
            
            if(attribute == "stroke_width"){
                node->circle.stroke_width = value;
            }
            if(attribute == "stroke_opacity"){
                node->circle.stroke_opacity = value;
            }
            if(attribute == "fill_opacity"){
                node->circle.fill_opacity = value;
            }
            
            break;
        case SVG_TAG_TYPE_RECT:
            if(attribute == "x"){
                node->rect.x = value;
            }
            if(attribute == "y"){
                node->rect.y = value;
            }
            if(attribute == "fill"){
                node->rect.fill = value;
            }
            if(attribute == "stroke_width"){
                node->rect.stroke_width = value;
            }
            if(attribute == "stroke"){
                node->rect.stroke = value;
            }
            if(attribute == "width"){
                node->rect.width = value;
            }
            if(attribute == "height"){
                node->rect.height = value;
            }
            if(attribute == "stroke_miterlimit"){
                node->rect.stroke_miterlimit = value;
            }
            if(attribute == "stroke_width"){
                node->rect.stroke_width = value;
            }
            if(attribute == "stroke_opacity"){
                node->rect.stroke_opacity = value;
            }
            if(attribute == "fill_opacity"){
                node->rect.fill_opacity = value;
            }
            
            break;
        case SVG_TAG_TYPE_PATH:
            // ofLog()<<"SVG_TAG_TYPE_PATH"<<endl;
            
            if(attribute == "fill"){
                node->path.fill = value;
            }
            if(attribute == "stroke_width"){
                node->path.stroke_width = value;
            }
            if(attribute == "stroke"){
                node->path.stroke = value;
            }
            if(attribute == "stroke_miterlimit"){
                node->path.stroke_miterlimit = value;
            }
            if(attribute == "stroke_width"){
                node->path.stroke_width = value;
            }
            if(attribute == "stroke_opacity"){
                node->path.stroke_opacity = value;
            }
            if(attribute == "fill_opacity"){
                node->path.fill_opacity = value;
            }
            
            
            break;
        case SVG_TAG_TYPE_GROUP:
            // ofLog()<<"SVG_TAG_TYPE_GROUP"<<endl;
            if(attribute == "transform"){
                node->group.transform = value;
            }
            if(attribute == "fill"){
                node->group.fill = value;
            }
            if(attribute == "stroke_width"){
                node->group.stroke_width = value;
            }
            if(attribute == "stroke"){
                node->group.stroke = value;
            }
            if(attribute == "stroke_miterlimit"){
                node->group.stroke_miterlimit = value;
            }
            if(attribute == "stroke_width"){
                node->group.stroke_width = value;
            }
            if(attribute == "stroke_opacity"){
                node->group.stroke_opacity = value;
            }
            if(attribute == "fill_opacity"){
                node->group.fill_opacity = value;
            }
            break;
        case SVG_TAG_TYPE_SVG:
            break;
        case SVG_TAG_TYPE_DOCUMENT:
            break;

    }
    
    
}

void ofxEditableSVG::setStrokeWidth(float f,int path){
    if(path>info.flatlist.size()-1){
        ofLog()<<"WARNING: Tried to modify ofxEditableSVG path out of index - "<< path<<" of info.flatlist.size() "<<info.flatlist.size()<<endl;
        return;
    }
    paths[path]->setStrokeWidth(f);
    setSVGattribute(info.flatlist[path],"stroke_width",ofToString(f));
    
};

void ofxEditableSVG::setStrokeColor(ofColor col,int path){
    if(path>info.flatlist.size()-1){
        ofLog()<<"WARNING: Tried to modify ofxEditableSVG path out of index - "<< path<<" of info.flatlist.size() "<<info.flatlist.size()<<endl;
        return;
    }
    paths[path]->setStrokeColor(col);
    //borg...update DOM
    setSVGattribute(info.flatlist[path],"stroke",hexToWeb(col));
    setSVGattribute(info.flatlist[path],"stroke_opacity",ofToString((float) col.a/255.0));
    
    
};

bool ofxEditableSVG::getFilled(int path){
    if(path>info.flatlist.size()-1){
        ofLog()<<"WARNING: Tried to modify ofxEditableSVG path out of index - "<< path<<" of info.flatlist.size() "<<info.flatlist.size()<<endl;
        return 0;
    }
    return paths[path]->isFilled();
};

ofColor ofxEditableSVG::getFillColor(int path){
    if(path>info.flatlist.size()-1){
        ofLog()<<"WARNING: Tried to modify ofxEditableSVG path out of index - "<< path<<" of info.flatlist.size() "<<info.flatlist.size()<<endl;
        return 0;
    }
    return paths[path]->getFillColor();
};

float ofxEditableSVG::getStrokeWidth(int path){
    if(path>info.flatlist.size()-1){
        ofLog()<<"WARNING: Tried to modify ofxEditableSVG path out of index - "<< path<<" of info.flatlist.size() "<<info.flatlist.size()<<endl;
        return 0;
    }
    return paths[path]->getStrokeWidth();
};

ofColor ofxEditableSVG::getStrokeColor(int path){
    if(path>info.flatlist.size()-1){
        ofLog()<<"WARNING: Tried to modify ofxEditableSVG path out of index - "<< path<<" of info.flatlist.size() "<<info.flatlist.size()<<endl;
        return 0;
    }
    return paths[path]->getStrokeColor();
};



//<image id="image1JPEG" x="240" y="0" width="240" height="150" xlink:href="data:image/jpg;base64,/9j/4AAQSkZJRgABAQAAAQABAAD/2w..."/>

void ofxEditableSVG::embedImage(ofImage &img, int _x, int _y, int w, int h, string uid,bool inNewGroup){
    ofImage imgCopy = img;
    if(imgCopy.getHeight() != h || imgCopy.getWidth()!= w){
        //reduce size if not all needed
        imgCopy.resize(w, h);
    }
    embedImage(imgCopy.getPixelsRef(),  _x,  _y,  w,  h,  uid , inNewGroup);
    imgCopy.clear();
};

void ofxEditableSVG::embedImage(ofPixelsRef &pix, int _x, int _y, int w, int h, string uid ,bool inNewGroup){


    ofBuffer buffer;
    ofSaveImage(pix, buffer, OF_IMAGE_FORMAT_PNG, OF_IMAGE_QUALITY_BEST);
 
    
    
    /*
    stringstream ss;
    ss<<"<image id=\""<<uid<<"\"";
    ss<<" x=\""<<ofToString(x)<<"\"";
    ss<<" y=\""<<ofToString(y)<<"\"";
    ss<<" width=\""<<ofToString(w)<<"\"";
    ss<<" height=\""<<ofToString(h)<<"\"";
    ss<<" xlink:href=\"data:image/png;base64,";
    Poco::Base64Encoder b64enc(ss);
    b64enc << buffer;
    b64enc.close();
    ss<<"\"/>";
    
    //to decode use
    ostringstream ostr;
    istringstream istr(source);

    Base64Decoder decoder(istr);
    StreamCopier::copyStream(decoder, ostr);
    ostr.str();
    */

    
     
    string rawXMLstring =  toString();
    
    Document * document;
    DOMParser parser;
    parser.setFeature(Poco::XML::XMLReader::FEATURE_NAMESPACE_PREFIXES, false);
    parser.setFeature(Poco::XML::XMLReader::FEATURE_NAMESPACES, false);
    
    
    document = parser.parseString(rawXMLstring);
    

    Element *imgNode = document->createElement("image");
    if(uid == "imgId"){
        imgNode->setAttribute("id",ofGetTimestampString());
    }else{
        imgNode->setAttribute("id",uid);
    }
    
    imgNode->setAttribute("x",ofToString(_x));
    imgNode->setAttribute("y",ofToString(_y));
    imgNode->setAttribute("width",ofToString(w));
    imgNode->setAttribute("height",ofToString(h));
    stringstream ss;
    Poco::Base64Encoder b64enc(ss);
    b64enc << buffer;
    b64enc.close();
    imgNode->setAttribute("xlink:href","data:image/png;base64,"+ss.str());

    
    ChildNodesList *cnl = (ChildNodesList *) document->childNodes();
    
    
    //go inside root
    Element *root = (Element *) cnl->item(0);
    
    if(inNewGroup){
        Element *groupElement = document->createElement("g");
        groupElement->setAttribute("id",ofGetTimestampString());
        groupElement->appendChild(imgNode);
        root->appendChild(groupElement);
    }else{
        root->appendChild(imgNode);
    }
    
    
    DOMWriter writer;
    stringstream xmlstream;
    writer.setNewLine("\n");
    writer.setOptions(XMLWriter::PRETTY_PRINT);
    writer.writeNode(xmlstream, document);
    
    //cout<<xmlstream.str()<<endl;

    parseXML(xmlstream.str());
};

/*
 From 08 ofPath has no subpaths but use Commands.
 Translate them into SVG XML nodes and append them to the document
 
 enum Type{
 moveTo,
 lineTo,
 curveTo,
 bezierTo,
 quadBezierTo,
 arc,
 arcNegative,
 close
 };
 
 
 Upper/lower case in d attrib refer to abs/relative position, eg. M/m
 
 A Bézier curve with one control point is called a quadratic Bézier curve and the kind with two control points is called cubic.
 
 Why this?!
 void ofPath::quadBezierTo(const ofPoint & cp1, const ofPoint & cp2, const ofPoint & p)
 */

void ofxEditableSVG::addPath(ofPath &path,bool inNewGroup){
    addPath(path,inNewGroup,"");
}
void ofxEditableSVG::addPath(ofPath &path,bool inNewGroup, string _groupName){
    
    string rawXMLstring =  toString();
    
    Document * document;
    DOMParser parser;
    parser.setFeature(Poco::XML::XMLReader::FEATURE_NAMESPACE_PREFIXES, false);
    parser.setFeature(Poco::XML::XMLReader::FEATURE_NAMESPACES, false);
    
    
    document = parser.parseString(rawXMLstring);
    
    //TODO: Check if has nodes else create new
    //Document *document = new Document();
    //parseHeader(document);
    
    
    
    
    //ofPtr<svgNode> childnode(new svgNode());
    //childnode->type = SVG_TAG_TYPE_PATH;
    
    
    //<path style=" stroke:none;fill-rule:nonzero;fill:rgb(0%,0%,0%);fill-opacity:1;" d="M 245.8125 37 C 245.8125 43.414062 236.1875 43.414062 236.1875 37 C 236.1875 30.585938 245.8125 30.585938 245.8125 37 Z M 245.8125 37 "/>
    

    
    Element *pathElement = parsePath(path,document);
    
    
    
    ChildNodesList *cnl = (ChildNodesList *) document->childNodes();
    
    
    //go inside root
    Element *root = (Element *) cnl->item(0);
    
    if(inNewGroup){
        ofLog()<<"inNewGroup id ";
        Element *groupElement = document->createElement("g");
        groupElement->setAttribute("inkscape-groupmode","layer");
        groupElement->setAttribute("id",ofGetTimestampString());
//        groupElement->setAttribute("stroke-width","0.123");
        groupElement->setAttribute("inkscape-label",_groupName);
        groupElement->appendChild(pathElement);
        root->appendChild(groupElement);
    }else{
        root->appendChild(pathElement);
    }
    
    
    DOMWriter writer;
    stringstream xmlstream;
    writer.setNewLine("\n");
    writer.setOptions(XMLWriter::PRETTY_PRINT);
    writer.writeNode(xmlstream, document);
    
    cout<<"xmlstream.str() "<<xmlstream.str();

    parseXML(xmlstream.str());
    
};

void ofxEditableSVG::addPaths(vector<ofPath> &paths,bool inNewGroup){

    
    string rawXMLstring =  toString();
    
    Document * document;
    DOMParser parser;
    parser.setFeature(Poco::XML::XMLReader::FEATURE_NAMESPACE_PREFIXES, false);
    parser.setFeature(Poco::XML::XMLReader::FEATURE_NAMESPACES, false);
    
    
    document = parser.parseString(rawXMLstring);
    
    ChildNodesList *cnl = (ChildNodesList *) document->childNodes();
    
    //go inside root
    Element *root = (Element *) cnl->item(0);
    
    
    
    Element * appendElement;
    if(inNewGroup){
        appendElement = document->createElement("g");
        appendElement->setAttribute("id",ofGetTimestampString());
    }else{
        appendElement = root;
    }
    
    for(int i=0;i<paths.size();i++){
        Element *pathElement = parsePath(paths[i],document);
        appendElement->appendChild(pathElement);
    }
    
    
    if(inNewGroup){
        root->appendChild(appendElement);
    }
    

    
    DOMWriter writer;
    stringstream xmlstream;
    writer.setNewLine("\n");
    writer.setOptions(XMLWriter::PRETTY_PRINT);
    writer.writeNode(xmlstream, document);
    
    
//    cout<<"xmlstream.str() "<<xmlstream.str();
    parseXML(xmlstream.str());
};


Element *ofxEditableSVG::parsePath(ofPath path,Document * document){
    Element *pathElement;;
    
    string d="";
    
    for(ofPath::Command com:path.getCommands()){
        
        //ofPath::Command::Type
        int t = com.type;
        switch (t){
            case 0:{
                //moveTo
                d+= " M"+ ofToString(com.to.x)+","+ofToString(com.to.y);
                pathElement = document->createElement("path");
                pathElement->setAttribute("d",d);
                break;
            }
            case 1:{
                //lineTo
                d+= " L"+ ofToString(com.to.x)+","+ofToString(com.to.y);
                pathElement = document->createElement("path");
                pathElement->setAttribute("d",d);
                break;
            }
            case 2:{
                //curveTo
                //TODO: THIS IS WRONG BUT OF IMPLEMENTS curveTo in an odd way as it's only asking for ONE point - which?
                ofLogWarning()<<"curveTo to SVG not implemented"<<endl;
                d+= " Q"+ofToString(com.cp1.x)+","+ofToString(com.cp1.y)+" "+ ofToString(com.to.x)+","+ofToString(com.to.y);
                pathElement = document->createElement("path");
                pathElement->setAttribute("d",d);
                break;
            }
            case 3:{
                //bezierTo
                //SVG C is cubic bezier
                d+= " C"+ofToString(com.cp1.x)+","+ofToString(com.cp1.y)+" "+ofToString(com.cp2.x)+","+ofToString(com.cp2.y)+" "+ ofToString(com.to.x)+","+ofToString(com.to.y);
                pathElement = document->createElement("path");
                pathElement->setAttribute("d",d);
                break;
            }
            case 4:{
                //quadBezierTo
                //SVG Q is quadratic bezier
                d+= " Q"+ofToString(com.cp1.x)+","+ofToString(com.cp1.y)+" "+ ofToString(com.to.x)+","+ofToString(com.to.y);
                pathElement = document->createElement("path");
                pathElement->setAttribute("d",d);
                break;
            }
            case 5:{
                //arc
                
                
                if((com.angleEnd - com.angleBegin)<360){
                    ofLogWarning()<<"arc to SVG not properly implemented"<<endl;
                    d+= " A"+ofToString(com.radiusX)+","+ofToString(com.radiusY)+" "+ ofToString(com.angleBegin)+" 1,0 "+ofToString(com.to.x) +","+ofToString(com.to.y);
                    
                    
                    
                    pathElement = document->createElement("path");
                    pathElement->setAttribute("d",d);
                }else{
                    
                    /*
                     <circle cx="100" cy="50" r="40" stroke="black"
                     stroke-width="2" fill="red"/>
                     */
                    
                    
                    //com.radiusX == com.radiusY
                    
                    
                    pathElement = document->createElement("circle");
                    pathElement->setAttribute("cx",ofToString(com.to.x));
                    pathElement->setAttribute("cy",ofToString(com.to.y));
                    pathElement->setAttribute("r",ofToString(com.radiusX));
                    
                }
                
                
                /*
                 rx ry x-axis-rotation large-arc-flag sweep-flag x y
                 */
                //TODO: Not tested
                //SVG Q is quadratic bezier
                
                
                
                
                break;
            }
            case 6:{
                //arcNegative
                break;
            }
            case 7:{
                //close
                d+= " Z ";
                pathElement = document->createElement("path");
                pathElement->setAttribute("d",d);
                break;
            }
        }
    }
    
    
    
    
    
    string style="";
    if(path.isFilled()){
        pathElement->setAttribute("fill",hexToWeb(path.getFillColor()));
        pathElement->setAttribute("fill-opacity",ofToString(path.getFillColor().a/255.0f));
    }else{
        pathElement->setAttribute("fill","none");
        
    }
    
    
    
    if(true){
        //ever not the case?
        pathElement->setAttribute("stroke",hexToWeb(path.getStrokeColor()));
        pathElement->setAttribute("stroke-width",ofToString(path.getStrokeWidth()));
        
        pathElement->setAttribute("stroke-opacity",ofToString(path.getStrokeColor().a/255.0f));
        
        
        if(path.getStrokeWidth()>0.1){
            pathElement->setAttribute("stroke-width",ofToString(path.getStrokeWidth()));
        }else{
            pathElement->setAttribute("stroke-width","1.0");
        }
        
    }else{
        pathElement->setAttribute("stroke","none");
        
    }
    
    return pathElement;
};


/*
 
 Trouble with insert is that it probably removes the original from the previous vector
 http://stackoverflow.com/questions/4122789/copy-object-keep-polymorphism/4122835#4122835
 
 Using pointers in vectors no need to make deep copy
 
 How do I copy translations from merged svg? By adding transformations to a group
 
 The svgs are put into groups.
 
 */
void ofxEditableSVG::merge(ofxEditableSVG & svg){
    //copy children
    ofPtr<svgNode> childnode(new svgNode());
    svgGroupDef gDef;
    gDef.transform = "translate("+ofToString(svg.getX())+","+ofToString(svg.getY())+")";
    
    childnode->group = gDef;
    childnode->type =SVG_TAG_TYPE_GROUP;
    
    
    
    childnode->children = svg.info.rootnode->children;
    info.rootnode->children.push_back(childnode);
    
    /*
     reparsing of the whole svg. If correct then all paths etc will be updated.
     */
    
    string str = toString();
    parseXML(str);
    
    
    
};

/*
 These values are not used to update paths.
 You need to manually adjust draw pos of whole svg if drawing paths.
 They are however stored in saved svg.
 */
void ofxEditableSVG::setPos(int _x, int _y){
    x = _x;
    y = _y;
};

void  ofxEditableSVG::setPos(ofPoint pt){
    x = pt.x;
    y = pt.y;
};

int ofxEditableSVG::getX(){
    return x;
};
int ofxEditableSVG::getY(){
    return y;
};


// //string because it includes unit, eg, 400pt
void ofxEditableSVG::setSize(int w, int h, string unit){
    width = w;
    height = h;
    info.width = ofToString(w)+unit;
    info.height = ofToString(h)+unit;
}


int ofxEditableSVG::getWidth(){
    return (int) width;
};


int ofxEditableSVG::getHeight(){
    return (int) height;
};



void ofxEditableSVG::setViewbox(int x, int y,int w, int h){
    info.viewbox = ofToString(x)+" " +ofToString(y) +" " +ofToString(w)+" " +ofToString(h);
    info.y = ofToString(y) +"px";
    info.x = ofToString(x) +"px";
    info.width = ofToString(w) +"px";
    info.height = ofToString(h) +"px";
    ofLog()<<"setViewbox() "<<info.width;
};

void ofxEditableSVG::save(string file){
    /*
     BORG
     https://jwatt.org/svg/authoring/#doctype-declaration
     
     Wrong DOCTTYPE upsets illustrator. Adviced to leave out.
     
     */
    
    
    Document *document = new Document();
    parseHeader(document);
    
	//DOMParser parser;
    //parser.setFeature(Poco::XML::XMLReader::FEATURE_NAMESPACES, false);
    
    //document = parser.parseString(info.rawXML);
    //svg = document->documentElement();
    
    
    //Poco::UTF8Encoding utf8encoding;
    //Poco::XML::XMLWriter writer(xmlstream, 0, "UTF-8", &utf8encoding);
    
    DOMWriter writer;
    
    
    /*
     BORG
     remove unwanted namespace ns1
     this doesn't actually work for some reason.
     
     writer.writeNode(xmlstream, svg);
     string rawXMLstring = xmlstream.str();
     
     Poco::XML::Document * cleanDocument;
     Poco::XML::DOMParser parser;
     parser.setFeature(Poco::XML::XMLReader::FEATURE_NAMESPACE_PREFIXES, false);
     parser.setFeature(Poco::XML::XMLReader::FEATURE_NAMESPACES, false);
     
     cleanDocument = parser.parseString(rawXMLstring);
     
     */
    
    
    
    
    
    /*
     
     #if defined(TARGET_OF_IPHONE)
     //equivalent of
     //ofxiPhoneGetDocumentsDirectory();
     string dataPath = "../Documents/";
     
     #else
     string dataPath = ofToDataPath("");
     #endif
     
     
     */
    string fullXmlFile;
    /* if(OF_TARGET_IPHONE){
         fullXmlFile = ofToDataPath("../Documents/"+file);
     }else{*/
         fullXmlFile = ofToDataPath(file);
      //}
    writer.setNewLine("\n");
    writer.setOptions(XMLWriter::PRETTY_PRINT);
    writer.writeNode(fullXmlFile, document);
    ofLog()<<"Saved "<<fullXmlFile<<endl;
}


string ofxEditableSVG::toString(){
    Document *document = new Document();
    
    parseHeader(document);
    DOMWriter writer;
    stringstream xmlstream;
    writer.setNewLine("\n");
    writer.setOptions(XMLWriter::PRETTY_PRINT);
    writer.writeNode(xmlstream, document);
    
    return xmlstream.str();
    
}

//these methods are unrelated to the addImage which refer to adding an image node
//these return the svg as an ofImage
ofImage ofxEditableSVG::getImage(int MSAA){
    
    MSAA = MIN(MSAA,4);
    int tw = getWidth()*MSAA;
    int th = getHeight()*MSAA;
        
    //adapt to max texture limit supported on device
    int max;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
    
    float MSAAcomp = 1.0f;
    
    if(tw>max && tw>th){
        MSAAcomp = max/(float)tw;
    }else if(th>max){
        MSAAcomp = max/(float)th;
    }
    
    tw*=MSAAcomp;
    th*=MSAAcomp;
    
    
    ofFbo fbo;
    ofImage img;

    
    fbo.allocate(tw, th,GL_RGBA);
    

    
    ofPushStyle();
    fbo.begin();
    ofClear(255, 0);
    ofEnableAlphaBlending();
    //ofBackground(bgColour);
    ofSetColor(255);
    ofScale(MSAA*MSAAcomp,MSAA*MSAAcomp);
    draw();
    
    
    fbo.end();
    ofPopStyle();
    
    
    
    ofPixels pixels;
    fbo.readToPixels(pixels);
    img.setFromPixels(pixels.getPixels(), fbo.getWidth(),fbo.getHeight(), OF_IMAGE_COLOR_ALPHA);
    
    if(MSAA>1){
        img.resize(getWidth(),getHeight());
    }
    return img;
    

};



ofImage ofxEditableSVG::getImage(int w, int h,ofColor bg,int MSAA){
    cout<<"ofxEditableSVG::getImage "<<w<<"x"<<h<<endl;
    MSAA = MIN(MSAA,4);
    
    //GL_MAX_TEXTURE_SIZE 4096
 
    
    int tw = w*MSAA;
    int th = h*MSAA;
    
    
    //adapt to max texture limit supported on device
    int max;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
    
    float MSAAcomp = 1.0f;
    
    if(tw>max && tw>th){
        MSAAcomp = max/(float)tw;
    }else if(th>max){
        MSAAcomp = max/(float)th;
    }
    
    tw*=MSAAcomp;
    th*=MSAAcomp;
    
    
    
    float sx = (float)w/(float)getWidth();
    float sy = (float)h/(float)getHeight();
    ofImage img;
    img.allocate(w, h,OF_IMAGE_COLOR);
    ofFbo fbo;


    fbo.allocate(tw, th,GL_RGBA);


    if(!fbo.isAllocated()){
        ofLogError()<<"ofxEditableSVG::getImage not even allocated tw, th "<<tw<<", "<<th<<endl;
    }
    
    ofPushStyle();
    fbo.begin();
    ofClear(255, 0);
    ofEnableAlphaBlending();
    ofBackground(bg);
    ofSetColor(255);
    ofScale(MSAA*MSAAcomp*sx,MSAA*MSAAcomp*sy);
    draw();
    fbo.end();
    ofPopStyle();
    
    
    
    ofPixels pixels;
    fbo.readToPixels(pixels);
    
    if(pixels.getPixels()){
        img.setFromPixels(pixels.getPixels(), fbo.getWidth(),fbo.getHeight(), OF_IMAGE_COLOR_ALPHA);
       // ok = true;
    }else{
        ofLogError()<<"ofImage ofxEditableSVG::getImage pixels.getPixels() returning null."<<endl;
    
    }
    if(MSAA>1){
        img.resize(w,h);
    }
    


    return img;
    
    
};



void ofxEditableSVG::parseHeader(Document * document){
    
    Element *svg = document->createElement("svg");
    if(!info.x.empty()){
        svg->setAttribute("x",info.x);
    }
    if(!info.y.empty()){
        svg->setAttribute("y",info.y);
    }
    if(!info.width.empty()){
        svg->setAttribute("width",info.width);
    }
    if(!info.height.empty()){
        svg->setAttribute("height",info.height);
    }
    if(!info.enable_background.empty()){
        svg->setAttribute("enable-background",info.enable_background);
    }
    if(!info.viewbox.empty()){
        svg->setAttribute("viewBox",info.viewbox);
    }
    if(!info.version.empty()){
        svg->setAttribute("version",info.version);
    }
    if(!info.id.empty()){
        svg->setAttribute("id",info.id);
    }
    if(!info.preserveAspectRatio.empty()){
        svg->setAttribute("preserveAspectRatio",info.preserveAspectRatio);
    }
    if(!info.xmlns.empty()){
        svg->setAttribute("xmlns",info.xmlns);
    }else{
        svg->setAttribute("xmlns","http://www.w3.org/2000/svg");
    }
    

    if(!info.xmlns_xlink.empty()){
        svg->setAttribute("xmlns:xlink",info.xmlns_xlink);
    }else{
        svg->setAttribute("xmlns:xlink","http://www.w3.org/1999/xlink");
    }
    
    //<!-- Generator: VectorSNAP  -->
    
    // ofLog()<<"info.preserveAspectRatio "<<info.preserveAspectRatio<<endl;
    //svg->setAttribute("xml:space",info.xml_space);
    //svg->setAttribute("xmlns",info.xmlns);
    //svg->setAttribute("xmlns:ns1",info.xmlns_ns1);
    //svg->setAttribute("xmlns:xlink",info.xmlns_xlink);
    
    
    document->appendChild(svg);
    
    if(info.rootnode){
        xmlCreateSVG(document,svg,info.rootnode);
    }else{
        ofLog()<<"check info.rootnode health"<<endl;
    }
    
    
    
    
};


Element * ofxEditableSVG::parseNode(Document *doc,ofPtr<svgNode> node){
    
    if(!node){
        ofLog()<<"Why is node null?"<<endl;
        return 0;
    }
    
    Element * newNode;
    switch (node->type){
        case SVG_TAG_TYPE_CIRCLE:
            newNode = doc->createElement("circle");
            // ofLog()<<"node->path.fill "<<node->path.fill<<endl;
            if(!node->circle.cx.empty()){
                newNode->setAttribute("cx",node->circle.cx);
            }
            if(!node->circle.cy.empty()){
                newNode->setAttribute("cy",node->circle.cy);
            }
            if(!node->circle.r.empty()){
                newNode->setAttribute("r",node->circle.r);
            }
            
            if(!node->circle.fill.empty()){
                newNode->setAttribute("fill",node->circle.fill);
            }
            if(!node->circle.stroke.empty()){
                newNode->setAttribute("stroke",node->circle.stroke);
            }
            
            if(!node->circle.stroke_width.empty()){
                newNode->setAttribute("stroke-width",node->circle.stroke_width);
            }
            if(!node->circle.fill_opacity.empty()){
                newNode->setAttribute("fill-opacity",node->circle.fill_opacity);
            }
            if(!node->circle.stroke_opacity.empty()){
                newNode->setAttribute("stroke-opacity",node->circle.stroke_opacity);
            }
            break;
        case SVG_TAG_TYPE_RECT:
            newNode = doc->createElement("rect");
            // ofLog()<<"node->path.fill "<<node->path.fill<<endl;
            if(!node->rect.x.empty()){
                newNode->setAttribute("x",node->rect.x);
            }
            if(!node->rect.y.empty()){
                newNode->setAttribute("y",node->rect.y);
            }
            if(!node->rect.width.empty()){
                newNode->setAttribute("width",node->rect.width);
            }
            if(!node->rect.height.empty()){
                newNode->setAttribute("height",node->rect.height);
            }
            if(!node->rect.fill.empty()){
                newNode->setAttribute("fill",node->rect.fill);
            }
            if(!node->rect.stroke.empty()){
                newNode->setAttribute("stroke",node->rect.stroke);
            }
            if(!node->rect.stroke_miterlimit.empty()){
                newNode->setAttribute("stroke-miterlimit",node->rect.stroke_miterlimit);
            }
            if(!node->rect.stroke_width.empty()){
                newNode->setAttribute("stroke-width",node->rect.stroke_width);
            }
            if(!node->rect.fill_opacity.empty()){
                newNode->setAttribute("fill-opacity",node->rect.fill_opacity);
            }
            if(!node->rect.stroke_opacity.empty()){
                newNode->setAttribute("stroke-opacity",node->rect.stroke_opacity);
            }
            break;
        case SVG_TAG_TYPE_IMAGE:
            newNode = doc->createElement("image");

            if(!node->image.x.empty()){
                newNode->setAttribute("x",node->image.x);
            }
            if(!node->image.y.empty()){
                newNode->setAttribute("y",node->image.y);
            }
            if(!node->image.width.empty()){
                newNode->setAttribute("width",node->image.width);
            }
            if(!node->image.height.empty()){
                newNode->setAttribute("height",node->image.height);
            }
            if(!node->image.base64ImgData.empty()){
                newNode->setAttribute("xlink:href",node->image.base64ImgData);
            }
            
            break;
        case SVG_TAG_TYPE_PATH:
            newNode = doc->createElement("path");
            newNode->setAttribute("d",node->path.d);
            //ofLog()<<"parse node->path.fill "<<node->path.fill<<endl;
            if(!node->path.fill.empty()){
                newNode->setAttribute("fill",node->path.fill);
            }
            if(!node->path.stroke.empty()){
                newNode->setAttribute("stroke",node->path.stroke);
            }
            if(!node->path.stroke_miterlimit.empty()){
                newNode->setAttribute("stroke-miterlimit",node->path.stroke_miterlimit);
            }
            if(!node->path.stroke_width.empty()){
                newNode->setAttribute("stroke-width",node->path.stroke_width);
            }
            if(!node->path.fill_opacity.empty()){
                newNode->setAttribute("fill-opacity",node->path.fill_opacity);
            }
            if(!node->path.stroke_opacity.empty()){
                newNode->setAttribute("stroke-opacity",node->path.stroke_opacity);
            }
            break;
        case SVG_TAG_TYPE_GROUP:
            // ofLog()<<"SVG_TAG_TYPE_GROUP"<<endl;
            newNode = doc->createElement("g");
            if(!node->group.transform.empty()){
                newNode->setAttribute("transform",node->group.transform);
            }
            if(!node->group.fill.empty()){
                newNode->setAttribute("fill",node->group.fill);
            }
            if(!node->group.stroke.empty()){
                newNode->setAttribute("stroke",node->group.stroke);
            }
            if(!node->group.stroke_miterlimit.empty()){
                newNode->setAttribute("stroke-miterlimit",node->group.stroke_miterlimit);
            }
            if(!node->group.stroke_width.empty()){
                newNode->setAttribute("stroke-width",node->group.stroke_width);
            }
            if(!node->group.fill_opacity.empty()){
                newNode->setAttribute("fill-opacity",node->group.fill_opacity);
            }
            if(!node->group.stroke_opacity.empty()){
                newNode->setAttribute("stroke-opacity",node->group.stroke_opacity);
            }
            if(!node->group.id.empty()){
                newNode->setAttribute("id",node->group.id);
            }
            if(!node->group.inkscape_label.empty()){
                newNode->setAttribute("inkscape-label",node->group.inkscape_label);
            }
            if(!node->group.inkscape_groupmode.empty()){
                newNode->setAttribute("inkscape-groupmode",node->group.inkscape_groupmode);
            }
            break;
        case SVG_TAG_TYPE_SVG:
            // ofLog()<<"SVG_TAG_TYPE_SVG"<<endl;
            newNode = doc->createElement("svg");
            newNode->setAttribute("x",node->svg.x);
            newNode->setAttribute("y",node->svg.y);
            newNode->setAttribute("width",node->svg.width);
            newNode->setAttribute("height",node->svg.height);
            if(!node->svg.enable_background.empty()){
                newNode->setAttribute("enable-background",node->svg.enable_background);
            }
            
            // newNode->setAttribute("viewBox",node->svg.viewbox);
            break;
        case SVG_TAG_TYPE_DOCUMENT:
            //ofLog()<<"SVG_TAG_TYPE_DOCUMENT"<<endl;
            //we are ignoring nested svg attributes ...its just that the doc root is xml root...and stuff....whatever
            return 0;
            /*
             newNode = doc->createElement("svg");
             newNode->setAttribute("x",node->svg.x);
             newNode->setAttribute("y",node->svg.y);
             newNode->setAttribute("width",node->svg.width);
             newNode->setAttribute("height",node->svg.height);
             if(!node->svg.enable_background.empty()){
             newNode->setAttribute("enable-background",node->svg.enable_background);
             }
             */
            break;
            
    }
    return newNode;
};

void ofxEditableSVG::xmlCreateSVG(Document *doc,Element *extNode, ofPtr<svgNode> intNode){
    Element * newNode = parseNode(doc,intNode);
    if(newNode!=0){
        extNode->appendChild(newNode);
        //recursive call
        for(int i=0;i<intNode->children.size();i++){
            xmlCreateSVG(doc,newNode,intNode->children[i]);
        }
    }else{
        //recursive call
        for(int i=0;i<intNode->children.size();i++){
            xmlCreateSVG(doc,extNode,intNode->children[i]);
        }
    }
    
    
    
};

/////////LOGICAL OPERATIONS

//return a vector of ofPolylines...I wish I could convert directly to
//svg but would need to port cairos subPath to SVG renderer to do so
//cairo not supported on iOS
/*
 
 ofPolylinesRef ofxEditableSVG::logicalIntersection(ofxEditableSVG & svg){
 //ofxEditableSVGRef newSVG(new ofxEditableSVG());
 generateClipper(this,&svg);
 // execute the clipping operation based on the current clipping type
 clipper.clip(OFX_CLIPPER_INTERSECTION,clips);
 
 ofPolylinesRef polyRef(new vector<ofPolyline>);
 
 for(int i=0;i<clips.size();i++){
 polyRef->push_back(clips[i]);
 }
 
 return polyRef;
 };
 
 ofPolylinesRef ofxEditableSVG::logicalDifference(ofxEditableSVG & svg){
 //ofxEditableSVGRef newSVG(new ofxEditableSVG());
 generateClipper(this,&svg);
 // execute the clipping operation based on the current clipping type
 clipper.clip(OFX_CLIPPER_DIFFERENCE,clips);
 
 ofPolylinesRef polyRef(new vector<ofPolyline>);
 
 for(int i=0;i<clips.size();i++){
 polyRef->push_back(clips[i]);
 }
 
 return polyRef;
 
 };
 
 ofPolylinesRef ofxEditableSVG::logicalUnion(ofxEditableSVG & svg){
 //ofxEditableSVGRef newSVG(new ofxEditableSVG());
 generateClipper(this,&svg);
 // execute the clipping operation based on the current clipping type
 clipper.clip(OFX_CLIPPER_UNION,clips);
 
 ofPolylinesRef polyRef(new vector<ofPolyline>);
 
 for(int i=0;i<clips.size();i++){
 polyRef->push_back(clips[i]);
 }
 
 return polyRef;
 };
 
 ofPolylinesRef ofxEditableSVG::logicalXor(ofxEditableSVG & svg){
 //ofxEditableSVGRef newSVG(new ofxEditableSVG());
 generateClipper(this,&svg);
 // execute the clipping operation based on the current clipping type
 clipper.clip(OFX_CLIPPER_XOR,clips);
 
 ofPolylinesRef polyRef(new vector<ofPolyline>);
 
 for(int i=0;i<clips.size();i++){
 polyRef->push_back(clips[i]);
 }
 
 return polyRef;
 
 };
 
 
 void ofxEditableSVG::generateClipper(ofxEditableSVG * src,ofxEditableSVG * clip){
 clips.clear();
 clipSubjects.clear();
 clipMasks.clear();
 clipper.clear();
 
 ofPoint pt;
 pt.set(src->getX(),src->getY());
 
 ofPath op;
 vector<ofPolyline> vp;
 for(int i=0;i<src->getNumPath();i++){
 op = src->getPathAt(i);
 op.translate(pt);
 vp = op.getOutline();
 
 clipSubjects.insert(clipSubjects.end(), vp.begin(), vp.end());
 
 }
 
 // add the clipper subjects (i.e. the things that will be clipped)
 clipper.addPolylines(clipSubjects,OFX_CLIPPER_SUBJECT);
 
 
 pt.set(clip->getX(),clip->getY());
 for(int i=0;i<clip->getNumPath();i++){
 op = clip->getPathAt(i);
 op.translate(pt);
 vp = op.getOutline();
 
 clipMasks.insert(clipMasks.end(), vp.begin(), vp.end());
 
 }
 
 
 
 // add the clipper masks (i.e. the things that will do the clipping)
 clipper.addPolylines(clipMasks,OFX_CLIPPER_CLIP);
 
 };
 
 */
