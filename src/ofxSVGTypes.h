/*
 *  ofxSVGTypes.h
 *  This helps storing the svg data structure internally so as to be
 *  able to manipulate and save out. The ofxSVG (port of ofxSVGTiny) only
 *  parses into paths with no memory from where it came...
 *
 *  Created by Andreas Borg on 16/10/2012
 *  Copyright 2012. All rights reserved.
 *
 */

#ifndef _ofxSVGTypes
#define _ofxSVGTypes

#include "ofMain.h"


struct svgNode;
typedef  enum SVG_TAG_TYPE{
    SVG_TAG_TYPE_CIRCLE,
    SVG_TAG_TYPE_RECT,
    SVG_TAG_TYPE_PATH,
    SVG_TAG_TYPE_IMAGE,
    SVG_TAG_TYPE_GROUP,
    SVG_TAG_TYPE_SVG,
    SVG_TAG_TYPE_DOCUMENT
}SVG_TAG_TYPE;


/*
 TODO: COMPLETE THESE FOR ALL FORMS CIRCLE, ELLIPSE etc....
 
 */
typedef struct svgCircleDef{
    string fill;
    string stroke;
    string fill_opacity;//float 0-1
    string stroke_opacity;//float 0-1
    string stroke_width;
    string cx;
    string cy;
    string r;
}svgCircleDef;


typedef struct svgPathDef{
    string fill;
    string stroke;
    string fill_opacity;//float 0-1
    string stroke_opacity;//float 0-1
    string stroke_width;
    string stroke_miterlimit;
    string d;
}svgPathDef;

typedef struct svgRectDef{
    string fill;
    string stroke;
    string fill_opacity;//float 0-1
    string stroke_opacity;//float 0-1
    string stroke_width;
    string stroke_miterlimit;
    string x;
    string y;
    string width;
    string height;
    
}svgRectDef;





typedef struct svgGroupDef {
    string transform;
    string fill;
    string stroke;
    string stroke_width;
    string stroke_miterlimit;
    string fill_opacity;//float 0-1
    string stroke_opacity;//float 0-1    
    // string rawXML;
}svgGroupDef;

typedef struct svgImageDef {
    string x;
    string y;
    string width;
    string height;
    string base64ImgData;
}svgImageDef;

typedef struct svgDef {
    string x;
    string y;
    string width;
    string height;
    string viewbox;
    string enable_background;
    //string rawXML;
}svgDef;



typedef struct svgNode{
    svgDef svg;
    svgGroupDef group;
    svgPathDef path;
    svgRectDef rect;
    svgCircleDef circle;
    svgImageDef image;
    vector<ofPtr<svgNode> >children;//
    SVG_TAG_TYPE type;
}svgNode;




/*
 version="1.1" id="Layer_1" xmlns:ns1="http://www.w3.org/2000/svg"
 xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px" width="360px" height="480px"
 viewBox="0 0 360 480" enable-background="new 0 0 360 480" xml:space="preserve"
 
 
 You can nest svgs inside svgs. 
 http://tutorials.jenkov.com/svg/svg-element.html
 
 
 The <g> element is used to group SVG shapes together. 
 Once grouped you can transform the whole group of shapes as if it was a single shape. 
 This is an advantage compared to a nested <svg> element which cannot be the target of transformation by itself. 
 
 http://tutorials.jenkov.com/svg/g-element.html
 
 So groups can have transforms but not x and y.
 If you need to move all shapes within a <g>-element using x and y attributes you will need 
 to nest the <g>-element inside an <svg>-element. The <svg>-element has x and y attributes.
 */
typedef struct svgInfo {
    ofPtr<svgNode> rootnode;//nested nodes reflecting original doc structure, used to save out mod doc
    vector<ofPtr<svgNode> >flatlist;//flatlist that corresponds to ofPath ids, used to update nodes when paths changed
    string version;
    string id;
    string xmlns;
    //   string xmlns_ns1;
    string xmlns_xlink;
    string x;
    string y;
    string width;
    string height;
    string viewbox;
    string enable_background;
    string xml_space;
    string preserveAspectRatio;
    string rawXML;
    
}svgInfo;





#endif
