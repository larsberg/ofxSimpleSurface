#pragma once

#include "ofMain.h"
#include "ofxSimpleSurface.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

	
	ofEasyCam camera;
	vector < vector< ofVec3f > > cv;
	ofxSimpleSurface surface;

	ofShader normalShader;
};
