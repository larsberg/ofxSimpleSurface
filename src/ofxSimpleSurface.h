//
//  ofxSimpleSurface.h
//  simpleSurface
//
//  Created by laserstorm on 10/27/13.
//
//

#pragma once
#include "ofMain.h"
#include "ofxSimpleSpline.h"

class ofxSimpleSurface{
public:
	ofxSimpleSurface();
	~ofxSimpleSurface();
	
	
	void clearCV();
	
	void setupUSplines();
	
	void setControlVertices( vector< vector<ofVec3f> >& _cv );
	
	void addControlVertices( vector< vector<ofVec3f> >& _cv );
	
	ofVec3f& getCV( int u, int v)
	{
		if(u<controlVertices->size() && v < (*controlVertices)[0].size() )	return (*controlVertices)[u][v];
		else return cv[0][0];
	}
	
	ofVec3f pointOnSurface( float u, float v);
	
	void setup( int _subdU=3, int _subdV=3);
	
	ofVec3f normalFrom3Points(ofVec3f p0, ofVec3f p1, ofVec3f p2);
	
	void updateNormals();
	
	void update();
	
	ofxSimpleSpline* getUHull(float v);
	
	ofVboMesh& getMesh();
	
	void draw();
	
	void drawSplines();
	
	
	//private:
	vector < vector< ofVec3f > >* controlVertices;
	vector < vector< ofVec3f > > cv;
	
	vector<ofxSimpleSpline> uSplines;
	ofxSimpleSpline uHull;
	float uHullPos;
	
	
	//mesh
	int subdU, subdV, numU, numV;
	float stepU, stepV;
	ofVboMesh mesh;
	
	vector<ofIndexType> indices;
	vector<ofVec3f> vertices, faceNormals, vertexNormals;
	vector<ofVec2f> texCoords;
	
	int numUControlVertices, numVControlVertices;
	
	
};
