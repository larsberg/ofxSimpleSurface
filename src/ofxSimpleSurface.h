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
	vector< vector<ofVec3f> >* getControlVertices()
	{
		return controlVertices;
	}
	
	
	void addControlVertices( vector< vector<ofVec3f> >& _cv );
	
	ofVec3f& getCV( int u, int v)
	{
		if(u<controlVertices->size() && v < (*controlVertices)[0].size() )	return (*controlVertices)[u][v];
		else return cv[0][0];
	}
	
	ofVec3f pointOnSurface( float u, float v);
	ofVec3f getSurfaceNormal( float u, float v);
	void getSurfacePositionAndNormal( ofVec3f& pos, ofVec3f& norm, float _u, float _v );
	ofVec3f pointOnSurface( ofVec2f uv){	return pointOnSurface(uv.x, uv.y);}
	ofVec3f getSurfaceNormal( ofVec2f uv){	return getSurfaceNormal(uv.x, uv.y);}
	
	ofVec4f getMeshFace(float u, float v);
	void getMeshPositionAndNormal( ofVec3f& pos, ofVec3f& norm, float _u, float _v);
	
	void setup( int _subdU=3, int _subdV=3);
	
	ofVec3f normalFrom3Points(ofVec3f p0, ofVec3f p1, ofVec3f p2);
	
	void updateNormals();
	
	void update( bool bUpdateNormals=true);
	
	ofxSimpleSpline* getUHull(float v);
	
	ofVboMesh& getMesh();
	
	void draw();
	void drawWireframe();
	
	void drawSplines();
	
	void flipNormals()
	{
		vector<ofIndexType>& indices = mesh.getIndices();
		
		int swapper;
		for(int i=0; i<indices.size(); i+=3)
		{
			swapper = indices[i+1];
			mesh.setIndex(i+1, indices[i+2]);
			mesh.setIndex(i+2, swapper);
		};
	}
	
	bool getFaceted()
	{
		return bFaceted;
	}
	
	void setFaceted( bool faceted )
	{
		bFaceted = faceted;
	}
	
	void setClosed( bool close_u,bool close_v )
	{
		uHull.close( close_u );
		bClosedU = close_u;
		
		//TODO: handle close_v
	}
	
	
	//private:
	vector < vector< ofVec3f > >* controlVertices;
	vector < vector< ofVec3f > > cv;
	
	vector<ofxSimpleSpline> uSplines;
	ofxSimpleSpline uHull;
	float uHullPos;
	
	bool bFaceted, bClosedU;
	
	
	//mesh
	int subdU, subdV, numU, numV;
	float stepU, stepV;
	ofVboMesh mesh, facetedMesh;
	
	vector<ofVec3f> vertices, faceNormals, vertexNormals;
	vector<ofVec2f> texCoords;
	
	int numUControlVertices, numVControlVertices;
};
