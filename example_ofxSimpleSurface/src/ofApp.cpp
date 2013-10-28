#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	//load our normal shader
	normalShader.load("normalShader");
	
	//create some control vertices
	int cvUCount = 14;
	int cvVCount = 7;
	
	cv.resize( cvUCount );
	for( int i=0; i<cv.size(); i++)
	{
		cv[i].resize(cvVCount);
		for(int j=0; j<cv[i].size(); j++)
		{
			cv[i][j].set( 60*(i - .5 * cvUCount), (j - .5 * cvVCount )*60, 0);
		}
	}
	
	//setup our surface
	surface.setControlVertices( cv );
	surface.setup(6,3);
}

//--------------------------------------------------------------
void ofApp::update()
{
	//move the control vertices around
	float elapsedTime = ofGetElapsedTimef(), h = 150, sampleVal, inverseScale, dist;
	ofVec2f samplePos( ofGetMouseX() - ofGetWidth()/2, ofGetHeight()/2 - ofGetMouseY() );
	samplePos *= .01;
	
	for (int i=0; i<cv.size(); i++)
	{
		for (int j=0; j<cv[i].size(); j++)
		{
			dist = ofVec2f(i-5,j-5).distance( samplePos );
			sampleVal = elapsedTime + dist;
			inverseScale = (1. - dist / 8.);
			cv[i][j].z = sin( sampleVal ) * cos( sampleVal ) * h * inverseScale;
		}
	}

	//update the surface geometry
	surface.update();
	
	//frame rate
	ofSetWindowTitle( ofToString( ofGetFrameRate() ) );
}

//--------------------------------------------------------------
void ofApp::draw()
{
	glEnable( GL_DEPTH_TEST );
	camera.begin();
	
	//draw our surface mesh
	normalShader.begin();
	surface.draw();
	normalShader.end();
	
	//draw control vertices
	for (int i=0; i<cv.size(); i++)
	{
		for (int j=0; j<cv[i].size(); j++)
		{
			ofDrawSphere( cv[i][j], 3 );
		}
	}
	
	camera.end();
}
