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
	facetedMesh.clear();
	
};


ofVec3f ofxSimpleSurface::pointOnSurface( float u, float v)
{
	getUHull( v - floor(v) );
	return  uHull.getPoint( u - floor(u) );
}

ofVec3f ofxSimpleSurface::getSurfaceNormal( float u, float v)
{	
	ofVec3f p0 = pointOnSurface(u,v);
	ofVec3f p1 = pointOnSurface(u,v + .005);
	ofVec3f p2 = pointOnSurface(u + .005,v);
	
	return normalFrom3Points(p0, p2, p1);
}

ofVec4f ofxSimpleSurface::getMeshFace(float u, float v)
{
	int i = ofMap(u - floor(u), 0, 1, 0, numU);
	int j = ofMap(v - floor(v), 0, 1, 0, numV);
	
	i %= numU;
	j %= numU;
	
	int i1 = (i + 1) % (numU);
	int j1 = (j + 1) % (numV);
	
	ofVec4f fi;//( numV*i+j, numV*i+j+1, numV*(i+1)+j+1, numV*(i+1)+j );
	fi.x = numV * i + j;
	fi.y = numV * i + j1;
	
	fi.z = numV * i1 + j1;
	fi.w = numV * i1 + j;
	
	return  fi;
}

void ofxSimpleSurface::getSurfacePositionAndNormal( ofVec3f& pos, ofVec3f& norm, float _u, float _v )
{
	_u -= floor(_u);
	_v -= floor(_v);
	
	float _u1 = _u + .01;
	float _v1 = _v + .01;
	
	_u1 -= floor(_u1);
	_v1 -= floor(_v1);
	
	getUHull( _v );
	pos = uHull.getPoint( _u );
	
	ofVec3f p0 = pos;
	ofVec3f p1 = uHull.getPoint( _u1 );
	ofVec3f p2 = pointOnSurface(_u, _v1 );
		
	norm = normalFrom3Points(p0, p1, p2);
}

void ofxSimpleSurface::getMeshPositionAndNormal( ofVec3f& pos, ofVec3f& norm, float u, float v)
{
	ofVec4f fi = getMeshFace( u, v );
	
	//get face percentage
	u = ofMap( u, 0, 1, 0, numU);
	v = ofMap( v, 0, 1, 0, numV);
	u -= floor(u);
	v -= floor(v);
	
	float mu = 1. - u;
	float mv = 1. - v;
	
	//ofVec3f p0 = mesh.getVertex( fi.x );
	//ofVec3f p1 = mesh.getVertex( fi.y );
	//ofVec3f p2 = mesh.getVertex( fi.z );
	//ofVec3f p3 = mesh.getVertex( fi.w );
	//
	////basically a 2D linear lattice
	//pos = (mu * p0 + u * p3) * mv + (mu * p1 + u * p2) * v;
	
	pos = (mu * mesh.getVertex(fi.x) + u * mesh.getVertex(fi.w)) * mv + (mu * mesh.getVertex(fi.y) + u * mesh.getVertex(fi.z)) * v;
	
	if(!bFaceted)
	{
		//ofVec3f n0 = ;
		//ofVec3f n1 = mesh.getNormal( fi.y );
		//ofVec3f n2 = mesh.getNormal( fi.z );
		//ofVec3f n3 = mesh.getNormal( fi.w );
		//norm = ((mu * n0 + u * n3) * mv + (mu * n1 + u * n2) * v).normalize();
		
		norm = ((mu * mesh.getNormal(fi.x) + u * mesh.getNormal(fi.w)) * mv + (mu * mesh.getNormal(fi.y) + u * mesh.getNormal(fi.z)) * v).normalize();
	}
	else
	{
		//just guessing here...
		if(u*u + v+v < 1.41421356237 )// ~= square root of two..
		{
			norm = normalFrom3Points(mesh.getVertex( fi.x ), mesh.getVertex( fi.z ), mesh.getVertex( fi.y ));
		}else{
			norm = normalFrom3Points(mesh.getVertex( fi.x ), mesh.getVertex( fi.w ), mesh.getVertex( fi.z ));
		}
	}
	
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
	}

	for(int i=0; i<indices.size(); i+=3)
	{
		fn = normalFrom3Points( v[indices[i]], v[indices[i+1]], v[indices[i+2]]);
		fIndex++;
		
		if(bFaceted)
		{	
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
		else
		{
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
	bFaceted? facetedMesh.draw() : mesh.draw();
}

void ofxSimpleSurface::drawWireframe()
{
	bFaceted? facetedMesh.drawWireframe() : mesh.drawWireframe();
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
