#pragma once
/*
 This library was rewritten by Joshua Noble from the ofxSVGTiny library.
 It was meant for one way parsing of an SVG file to native ofPaths. 
 
 I have added the ability to modify the colour, visibility and line width of those paths
 and then save the new SVG out again. No provision is made for modifying the curve shapes,
 but it could be implemented along the same lines.
 
 This makes it possible to modify and save vector graphcis also on iOS.
 
Licensed under the MIT License,
http://opensource.org/licenses/mit-license.php
 
 
Copyright (2015) Andreas Borg
http://crea.tion.to
 
 */
#include "ofMain.h"
#include "svgtiny.h"
#include "ofPath.h"
#include "ofTypes.h"






#include "Poco/URIStreamOpener.h"  
#include "Poco/StreamCopier.h"  
#include "Poco/Path.h"  
#include "Poco/URI.h"  
#include "Poco/Exception.h"  
#include "Poco/Net/HTTPStreamFactory.h"  
#include "Poco/XML/XMLString.h"  
#include "Poco/DOM/DOMParser.h"  
#include "Poco/DOM/Document.h"  
#include "Poco/DOM/Attr.h"  
#include "Poco/DOM/NodeIterator.h"  
#include "Poco/DOM/NodeFilter.h"  
#include "Poco/DOM/NamedNodeMap.h"   
#include "Poco/DOM/ChildNodesList.h"
//BORG
#include "Poco/DOM/DOMWriter.h"
//#include "Poco/XML/XMLReader.h" 
#include "Poco/XML/XMLWriter.h" 
#include "Poco/UTF8Encoding.h"
#include "ofxSVGTypes.h"


//#include "ofxClipper.h"


using namespace Poco::XML;




class ofxEditableSVG {
	public: 
         ofxEditableSVG();
        ~ofxEditableSVG();


		float getWidth() const {
			return width;
		}
		float getHeight() const {
			return height;
		}
		void load(string path);
		void draw();

        int getNumPath();
        ofPath & getPathAt(int n);        
    
    
        //borg mod
        //applies to whole shape
        void setFilled(bool t);
        void setFillColor(ofColor col);
        void setStrokeWidth(float f);
        void setStrokeColor(ofColor col);//use to set alpha too
        bool getFilled();
        ofColor getFillColor();
        float getStrokeWidth();
        ofColor getStrokeColor();
    
    
        //applies to specific path
        void setFilled(bool t,int path);
        void setFillColor(ofColor col,int path);//use to set alpha too
        void setStrokeWidth(float f,int path);
        void setStrokeColor(ofColor col,int path);//use to set alpha too
        bool getFilled(int path);
        ofColor getFillColor(int path);
        float getStrokeWidth(int path);
        ofColor getStrokeColor(int path);
    
        string hexToWeb(ofColor col){
            return "#"+ofToHex(col.r)+ofToHex(col.g)+ofToHex(col.b);
        }
    
    
    
        //Functions to build new SVGs
        //reparse is required to update doc after changes, but when adding many
        //paths better to do after all is said and done
        void addPath(ofPath &path,bool inNewGroup=false);
        void addPaths(vector<ofPath> &paths,bool inNewGroup=false);
    
    
        void merge(ofxEditableSVG & svg);//add to current
        void save(string file);
        string toString();//return the whole svg as a xml document string..have fun

        //experimental support for embedding a base64 image in output.
        //internal rendering and parsing of embedded images in src svg
        //not yet supported (feel free to get yer hands dirty)
        //this is just to be able to embed ofImage generated content
        /*
        Usage example
        ofImage img;
        img.loadImage("red.jpg");
        svg.load("illustration.svg");
        svg.embedImage(img, 10,100, 100, 100,"myImg");
        svg.save("Merged_"+ofToString(ofGetTimestampString())+".svg");
        
        */
        void embedImage(ofImage &img, int x, int y, int w, int h, string uid ="imgId",bool inNewGroup=true);
        void embedImage(ofPixelsRef &pix, int x, int y, int w, int h, string uid ="imgId",bool inNewGroup=true);
    
        svgInfo info;
        typedef ofPtr <ofPath> ofPathRef;//note this! Shorthand for paths as ofPtrs
        vector <ofPathRef> paths;
        void parseXML(string xml);
        
    //use when creating new ofxEditableSVG
   
        void setSize(int w, int h, string unit="px");
        void setPos(ofPoint pt);
        void setPos(int x, int y);
        int getX();
        int getY();
        int getWidth();
        int getHeight();
        void setViewbox(int x, int y,int w, int h);
    
    
        //return as ofImage
        ofImage getImage(int MSAA = 1);
        ofImage getImage(int w, int h,ofColor bg, int MSAA = 1);
    
    //ofPath:tesselator crashes when too many commands, test for more than 10000
    void setComplexityThreshold(int i){
        complexityThreshold = i;
    }
    int getComplexityThreshold(){
        return complexityThreshold;
    }
    /*
    //logical operations
    ofPtr <vector <ofPolyline> > logicalIntersection(ofxEditableSVG & svg);
    ofPtr <vector <ofPolyline> > logicalDifference(ofxEditableSVG & svg);
    ofPtr <vector <ofPolyline> > logicalUnion(ofxEditableSVG & svg);
    ofPtr <vector <ofPolyline> > logicalXor(ofxEditableSVG & svg);
    */
    
	private:
    
        int complexityThreshold;//to avoid tesselator crash
    
        void parseHeader(Document *);
        void xmlCreateSVG(Document *,Element *,ofPtr<svgNode> );
        Element * parseNode(Document *doc,ofPtr<svgNode> );
        void setSVGattribute(ofPtr<svgNode>  node,string attribute, string value);
        bool isFilled;
        ofColor fill;
        ofColor stroke;
        float strokeWidth;

    
        //void updatePath(
    
		float width, height;
        float x,y;


		void setupDiagram(struct svgtiny_diagram * diagram);
		void setupShape(struct svgtiny_shape * shape);
    
    
    Element *parsePath(ofPath path,Document * document);
    /*
    ofxClipper clipper;
    ofxPolylines clipMasks;
    ofxPolylines clipSubjects;
    ofxPolylines clips;
    ofxPolylines offsets;
    void generateClipper(ofxEditableSVG* src,ofxEditableSVG * clip);
*/
};


typedef ofPtr <ofxEditableSVG> ofxEditableSVGRef;
typedef ofPtr <vector <ofPolyline> > ofPolylinesRef;
