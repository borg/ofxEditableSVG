/*
 * This file is part of Libsvgtiny
 * Licensed under the MIT License,
 *                http://opensource.org/licenses/mit-license.php
 * Copyright 2008 James Bursa <james@semichrome.net>
 
 
 
 SVG Path - <path>
 
 The <path> element is used to define a path.
 
 The following commands are available for path data:
 
 M = moveto
 L = lineto
 H = horizontal lineto
 V = vertical lineto
 C = curveto
 S = smooth curveto
 Q = quadratic Bézier curve
 T = smooth quadratic Bézier curveto
 A = elliptical Arc
 Z = closepath
 
 Note: All of the commands above can also be expressed with lower letters. Capital letters means absolutely positioned, lower cases means relatively positioned.
 
 
 
 */

#ifndef SVGTINY_H
#define SVGTINY_H

#include "Poco/URIStreamOpener.h"  
#include "Poco/StreamCopier.h"  
#include "Poco/Path.h"  
#include "Poco/URI.h"  
#include "Poco/Exception.h"  
#include "Poco/Net/HTTPStreamFactory.h"  
#include "Poco/XML/XMLString.h"  
#include "Poco/DOM/Element.h"  
#include "Poco/DOM/DOMParser.h"  
#include "Poco/DOM/Document.h"  
#include "Poco/DOM/Attr.h"  
#include "Poco/DOM/NodeIterator.h"  
#include "Poco/DOM/NodeFilter.h"  
#include "Poco/DOM/NamedNodeMap.h"   
#include "Poco/DOM/ChildNodesList.h"

#include <iostream>
#include <string>

using Poco::URIStreamOpener;  
using Poco::StreamCopier;  
using Poco::Path;  
using Poco::URI;  
using Poco::Exception;  
using Poco::Net::HTTPStreamFactory;
//using Poco::XML;
//using Poco::XML::XMLString;
//using Poco::XML::DOMParser;  
//using Poco::XML::Document;  
//using Poco::XML::NodeIterator;  
//using Poco::XML::NodeFilter;  
//using Poco::XML::Node;  
//using Poco::XML::NamedNodeMap; 




//BORG
#include "ofMain.h"
#include "Poco/DOM/DOMWriter.h"
//#include "Poco/XML/XMLReader.h" 
#include "Poco/XML/XMLWriter.h" 
#include "Poco/UTF8Encoding.h"

//is this ok?
//#include "ofxSVG.h"


#include "ofxSVGTypes.h"

///////

typedef int svgtiny_colour;
#define svgtiny_TRANSPARENT 0x1000000
#ifdef __riscos__
#define svgtiny_RGB(r, g, b) ((b) << 16 | (g) << 8 | (r))
#define svgtiny_RED(c) ((c) & 0xff)
#define svgtiny_GREEN(c) (((c) >> 8) & 0xff)
#define svgtiny_BLUE(c) (((c) >> 16) & 0xff)
#else
#define svgtiny_RGB(r, g, b) ((r) << 16 | (g) << 8 | (b))
#define svgtiny_RED(c) (((c) >> 16) & 0xff)
#define svgtiny_GREEN(c) (((c) >> 8) & 0xff)
#define svgtiny_BLUE(c) ((c) & 0xff)
#endif

struct svgtiny_shape {
	float *path;
	unsigned int path_length;
	char *text;
	float text_x, text_y;
	svgtiny_colour fill;
	svgtiny_colour stroke;
	int stroke_width;
};

struct svgtiny_diagram {
	int width, height;

	struct svgtiny_shape *shape;
	unsigned int shape_count;

	unsigned short error_line;
	const char *error_message;
};


//+borg
using namespace Poco::XML;
using namespace std;




//-borg


typedef enum {
	svgtiny_OK,
	svgtiny_OUT_OF_MEMORY,
	svgtiny_LIBXML_ERROR,
	svgtiny_NOT_SVG,
	svgtiny_SVG_ERROR
} svgtiny_code;

enum {
	svgtiny_PATH_MOVE,
	svgtiny_PATH_CLOSE,
	svgtiny_PATH_LINE,
	svgtiny_PATH_BEZIER
};

struct svgtiny_named_color {
	const char *name;
	svgtiny_colour color;
};


struct svgtiny_diagram *svgtiny_create(void);
svgtiny_code svgtiny_parse(svgInfo &info, struct svgtiny_diagram *diagram,const char *buffer,int width, int height);
void svgtiny_free(struct svgtiny_diagram *svg);


svgtiny_code processChildren(svgInfo &info,ofPtr<svgNode> node, Poco::XML::ChildNodesList *cnl,
                             struct svgtiny_parse_state state);



#endif
