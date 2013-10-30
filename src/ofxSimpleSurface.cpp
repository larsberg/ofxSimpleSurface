//
//  ofxSimpleSurface.cpp
//  simpleSurface
//
//  Created by lars berg on 10/27/13.
//
//

#include "ofxSimpleSurface.h"

ofxSimpleSurface::ofxSimpleSurface()
{
	cv.resize(1);
	cv[0].resize(1);
	
	controlVertices = &cv;
	
	subdU = 3;
	subdV = 3;
	
	bFaceted = false;
	bClosedU = false;
};
ofxSimpleSurface::~ofxSimpleSurface()
{
	clearCV();
	mesh.clear();
};


ofVec3f ofxSimpleSurface::pointOnSurface( float u, float v)
{
	getUHull( v );
	return  uHull.getPoint( u );
}

void ofxSimpleSurface::setControlVertices( vector< vector<ofVec3f> >& _cv )
{
	//clearCV();
	controlVertices = &_cv;
	
	setupUSplines();
}

void ofxSimpleSurface::addControlVertices( vector< vector<ofVec3f> >& _cv )
{
	clearCV();
	
	cv.resize(_cv.size());
	for(int i=0; i<_cv.size(); i++)
	{
		cv[i].resize(_cv[i].size());
		for(int j=0; j<_cv[i].size();j++)
		{
			cv[i][j] = _cv[i][j];
		}
	}
	
	setupUSplines();
}

void ofxSimpleSurface::clearCV()
{
	for(int i=0; i<cv.size(); i++)
	{
		cv[i].clear();
	}
	cv.clear();
}

void ofxSimpleSurface::setupUSplines()
{
	uHull.clear();
	uSplines.resize( controlVertices->size() );
	for( int i=0; i<controlVertices->size(); i++)
	{
		uSplines[i].setControlVertices( (*controlVertices)[i] );
		uHull.addControlVertex( ofVec3f() );
	}
}

void ofxSimpleSurface::setup( int _subdU, int _subdV)
{
	subdU = _subdU;
	subdV = _subdV;
	
	numU = subdU * uSplines.size();
	numV = subdV * uSplines[0].getControlVertices().size();
	
	stepU = 1. / float(numU-1);
	stepV = 1. / float(numV-1);
	
	vertices.resize( numU * numV );
	texCoords.resize( vertices.size() );
	
	//vertices
	float u, v;
	for(int i=0; i<numU; i++)
	{
		u = float(i) * stepU;
		for(int j=0; j<numV; j++)
		{
			v = float(j) * stepV;
			texCoords[numV * i + j].set( u, v );
			vertices[numV * i + j] = pointOnSurface(u,v);
		}
	}
	
	//indices	
	vector<ofIndexType> indices;

	for(int i=0; i<numU-1; i++)
	{
		for(int j=0; j<numV-1; j++)
		{
			indices.push_back(numV * i + j);
			indices.push_back(numV * (i+1) + j + 1);
			indices.push_back(numV * i + j + 1);
			
			indices.push_back(numV * i + j);
			indices.push_back(numV * (i+1) + j);
			indices.push_back(numV * (i+1) + j + 1);
		}
	}
	
	//normal arrays
	faceNormals.resize( indices.size() / 3 );
	vertexNormals.resize( vertices.size() );
	
	//build our mesh
	mesh.clear();
	mesh.addVertices( vertices );
	mesh.addNormals( vertexNormals );
	mesh.addTexCoords( texCoords );
	mesh.addIndices( indices );
	
	updateNormals();
}

ofVec3f ofxSimpleSurface::normalFrom3Points(ofVec3f p0, ofVec3f p1, ofVec3f p2)
{
	return (p2 - p1).cross( p0 - p1).normalized();
}

void ofxSimpleSurface::updateNormals()
{
	ofVec3f fn;
	vector<ofVec3f>& v = mesh.getVertices();
	vector<ofVec3f>& n = mesh.getNormals();
	vector<ofVec2f>& tc = mesh.getTexCoords();
	vector<ofIndexType>& indices = mesh.getIndices();
	
	fill (vertexNormals.begin(),vertexNormals.end(), ofVec3f(0,0,0));
	
	int fIndex = 0;
	if(bFaceted)
	{
		facetedMesh.getVertices().resize(indices.size());
		facetedMesh.getNormals().resize(indices.size());
		facetedMesh.getTexCoords().resize(indices.size());
		facetedMesh.getIndices().resize(indices.size());
	}

	for(int i=0; i<indices.size(); i+=3)
	{
		fn = normalFrom3Points( v[indices[i]], v[indices[i+1]], v[indices[i+2]]);
		faceNormals[fIndex] = fn;
		fIndex++;
		
		if(bFaceted)
		{
			facetedMesh.setIndex(i, i);
			facetedMesh.setIndex(i+1, i+1);
			facetedMesh.setIndex(i+2, i+2);
			
			facetedMesh.setVertex(i, v[indices[i]]);
			facetedMesh.setVertex(i+1, v[indices[i+1]]);
			facetedMesh.setVertex(i+2, v[indices[i+2]]);
			
			facetedMesh.setTexCoord(i, tc[indices[i]]);
			facetedMesh.setTexCoord(i+1, tc[indices[i+1]]);
			facetedMesh.setTexCoord(i+2, tc[indices[i+2]]);
			
			facetedMesh.setNormal(i, fn);
			facetedMesh.setNormal(i+1, fn);
			facetedMesh.setNormal(i+2, fn);
		}
		else{
			vertexNormals[indices[i]] += fn;
			vertexNormals[indices[i+1]] += fn;
			vertexNormals[indices[i+2]] += fn;
		}
	}
	
	if(!bFaceted)
	{
		if(bClosedU)
		{
			ofVec3f n;
			for(int j=0; j<numV; j++)
			{
				n = vertexNormals[j] + vertexNormals[numV*(numU-1)+j];
				vertexNormals[j] = n;
				vertexNormals[numV*(numU-1)+j] = n;
			}
		}
		for(int i=0; i<vertexNormals.size(); i++)
		{
			vertexNormals[i].normalize();
			mesh.setNormal(i, vertexNormals[i] );
		}
	}
}

void ofxSimpleSurface::update(bool bUpdateNormals)
{
	//vertex positions
	for (int i=0; i<numV; i++)
	{
		getUHull( float(i) * stepV );
		for (int j=0; j<numU; j++)
		{
			mesh.setVertex( numV * j + i, uHull.getPoint( float(j) * stepU ) );
		}
	}
	
	//normals
	if(bUpdateNormals)	updateNormals();
}

void ofxSimpleSurface::draw()
{
	if(bFaceted)
	{
		facetedMesh.draw();
	}
	else
	{
		mesh.draw();
	}
}

void ofxSimpleSurface::drawSplines()
{
	for(int i=0; i<uSplines.size(); i++)
	{
		uSplines[i].update();
		uSplines[i].draw();
	}
}

ofVboMesh& ofxSimpleSurface::getMesh()
{
	
	return bFaceted ? facetedMesh : mesh;
}

ofxSimpleSpline* ofxSimpleSurface::getUHull(float v)
{
	if(uHullPos != v)
	{
		uHullPos = v;
		vector<ofVec3f>& uHullCV = uHull.getControlVertices();
		for (int i=0; i<uHullCV.size(); i++) {
			uHullCV[i] = uSplines[i].getPoint(v);
		}
	}
	
	return  &uHull;
}
