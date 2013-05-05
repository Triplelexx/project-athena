//
//  VoxelNode.cpp
//  hifi
//
//  Created by Stephen Birarda on 3/13/13.
//
//

#include <stdio.h>
#include <cmath>
#include <cstring>
#include "SharedUtil.h"
#include "voxels_Log.h"
#include "VoxelNode.h"
#include "VoxelConstants.h"
#include "OctalCode.h"
#include "AABox.h"
using voxels_lib::printLog;

// using voxels_lib::printLog;

VoxelNode::VoxelNode() {
    unsigned char* rootCode = new unsigned char[1];
    *rootCode = 0;
    init(rootCode);
}

VoxelNode::VoxelNode(unsigned char * octalCode) {
    init(octalCode);
}

void VoxelNode::init(unsigned char * octalCode) {
    _octalCode = octalCode;
    
#ifdef HAS_FALSE_COLOR
    _falseColored = false; // assume true color
#endif
    
    // default pointers to child nodes to NULL
    for (int i = 0; i < 8; i++) {
        _children[i] = NULL;
    }
    
    _glBufferIndex = GLBUFFER_INDEX_UNKNOWN;
    _isDirty = true;
    _shouldRender = false;
    
    calculateAABox();
}

VoxelNode::~VoxelNode() {
    delete[] _octalCode;
    
    // delete all of this node's children
    for (int i = 0; i < 8; i++) {
        if (_children[i]) {
            delete _children[i];
        }
    }
}

void VoxelNode::setShouldRender(bool shouldRender) {
    // if shouldRender is changing, then consider ourselves dirty
    if (shouldRender != _shouldRender) {
        _shouldRender = shouldRender;
        _isDirty = true;
    }
}

void VoxelNode::calculateAABox() {
    
    glm::vec3 corner;
    glm::vec3 size;
    
    // copy corner into box
    copyFirstVertexForCode(_octalCode,(float*)&corner);
    
    // this tells you the "size" of the voxel
    float voxelScale = 1 / powf(2, *_octalCode);
    size = glm::vec3(voxelScale,voxelScale,voxelScale);
    
    _box.setBox(corner,size);
}

void VoxelNode::deleteChildAtIndex(int childIndex) {
    if (_children[childIndex]) {
        delete _children[childIndex];
        _children[childIndex] = NULL;
    }
}

void VoxelNode::addChildAtIndex(int childIndex) {
    if (!_children[childIndex]) {
        _children[childIndex] = new VoxelNode(childOctalCode(_octalCode, childIndex));
    
        // XXXBHG - When the node is constructed, it should be cleanly set up as 
        // true colored, but for some reason, not so much. I've added a a basecamp
        // to-do to research this. But for now we'll use belt and suspenders and set
        // it to not-false-colored here!
        _children[childIndex]->setFalseColored(false);
    
        _isDirty = true;
    }
}

// will average the child colors...
void VoxelNode::setColorFromAverageOfChildren() {
	int colorArray[4] = {0,0,0,0};
	for (int i = 0; i < 8; i++) {
		if (_children[i] != NULL && _children[i]->isColored()) {
			for (int j = 0; j < 3; j++) {
				colorArray[j] += _children[i]->getTrueColor()[j]; // color averaging should always be based on true colors
			}
			colorArray[3]++;
		}
	}
    
    nodeColor newColor = { 0, 0, 0, 0};
    if (colorArray[3] > 4) {
        // we need at least 4 colored children to have an average color value
        // or if we have none we generate random values
        for (int c = 0; c < 3; c++) {
            // set the average color value
            newColor[c] = colorArray[c] / colorArray[3];
        }
        // set the alpha to 1 to indicate that this isn't transparent
        newColor[3] = 1;
    }
    // actually set our color, note, if we didn't have enough children
    // this will be the default value all zeros, and therefore be marked as
    // transparent with a 4th element of 0
    setColor(newColor);
}

// Note: !NO_FALSE_COLOR implementations of setFalseColor(), setFalseColored(), and setColor() here.
//       the actual NO_FALSE_COLOR version are inline in the VoxelNode.h
#ifndef NO_FALSE_COLOR // !NO_FALSE_COLOR means, does have false color
void VoxelNode::setFalseColor(colorPart red, colorPart green, colorPart blue) {
    if (_falseColored != true || _currentColor[0] != red || _currentColor[1] != green || _currentColor[2] != blue) {
        _falseColored=true;
        _currentColor[0] = red;
        _currentColor[1] = green;
        _currentColor[2] = blue;
        _currentColor[3] = 1; // XXXBHG - False colors are always considered set
        //if (_shouldRender) {
            _isDirty = true;
        //}
    }
}

void VoxelNode::setFalseColored(bool isFalseColored) {
    if (_falseColored != isFalseColored) {
        // if we were false colored, and are no longer false colored, then swap back
        if (_falseColored && !isFalseColored) {
            memcpy(&_currentColor,&_trueColor,sizeof(nodeColor));
        }
        _falseColored = isFalseColored; 
        //if (_shouldRender) {
            _isDirty = true;
        //}
    }
};


void VoxelNode::setColor(const nodeColor& color) {
    if (_trueColor[0] != color[0] || _trueColor[1] != color[1] || _trueColor[2] != color[2]) {
        //printLog("VoxelNode::setColor() was: (%d,%d,%d) is: (%d,%d,%d)\n",
        //    _trueColor[0],_trueColor[1],_trueColor[2],color[0],color[1],color[2]);
        memcpy(&_trueColor,&color,sizeof(nodeColor));
        if (!_falseColored) {
            memcpy(&_currentColor,&color,sizeof(nodeColor));
        }
        //if (_shouldRender) {
            _isDirty = true;
        //}
    }
}
#endif

// will detect if children are leaves AND the same color
// and in that case will delete the children and make this node
// a leaf, returns TRUE if all the leaves are collapsed into a 
// single node
bool VoxelNode::collapseIdenticalLeaves() {
	// scan children, verify that they are ALL present and accounted for
	bool allChildrenMatch = true; // assume the best (ottimista)
	int red,green,blue;
	for (int i = 0; i < 8; i++) {
		// if no child, or child doesn't have a color
		if (_children[i] == NULL || !_children[i]->isColored()) {
			allChildrenMatch=false;
			//printLog("SADNESS child missing or not colored! i=%d\n",i);
			break;
		} else {
			if (i==0) {
				red   = _children[i]->getColor()[0];
				green = _children[i]->getColor()[1];
				blue  = _children[i]->getColor()[2];
			} else if (red != _children[i]->getColor()[0] || 
                    green != _children[i]->getColor()[1] || blue != _children[i]->getColor()[2]) {
				allChildrenMatch=false;
				break;
			}
		}
	}
	
	
	if (allChildrenMatch) {
		//printLog("allChildrenMatch: pruning tree\n");
		for (int i = 0; i < 8; i++) {
			delete _children[i]; // delete all the child nodes
			_children[i]=NULL; // set it to NULL
		}
		nodeColor collapsedColor;
		collapsedColor[0]=red;		
		collapsedColor[1]=green;		
		collapsedColor[2]=blue;		
		collapsedColor[3]=1;	// color is set
		setColor(collapsedColor);
	}
	return allChildrenMatch;
}

void VoxelNode::setRandomColor(int minimumBrightness) {
    nodeColor newColor;
    for (int c = 0; c < 3; c++) {
        newColor[c] = randomColorValue(minimumBrightness);
    }
    
    newColor[3] = 1;
    setColor(newColor);
}

bool VoxelNode::isLeaf() const {
    for (int i = 0; i < 8; i++) {
        if (_children[i]) {
            return false;
        }
    }
    return true;
}

void VoxelNode::printDebugDetails(const char* label) const {
    printLog("%s - Voxel at corner=(%f,%f,%f) size=%f octcode=", label,
        _box.getCorner().x, _box.getCorner().y, _box.getCorner().z, _box.getSize().x);
    printOctalCode(_octalCode);
}

bool VoxelNode::isInView(const ViewFrustum& viewFrustum) const {
    AABox box = _box; // use temporary box so we can scale it
    box.scale(TREE_SCALE);
    bool inView = (ViewFrustum::OUTSIDE != viewFrustum.boxInFrustum(box));
    return inView;
}

float VoxelNode::distanceToCamera(const ViewFrustum& viewFrustum) const {
    glm::vec3 center = _box.getCenter() * (float)TREE_SCALE;
    float distanceToVoxelCenter = sqrtf(powf(viewFrustum.getPosition().x - center.x, 2) +
                                        powf(viewFrustum.getPosition().y - center.y, 2) +
                                        powf(viewFrustum.getPosition().z - center.z, 2));
    return distanceToVoxelCenter;
}
