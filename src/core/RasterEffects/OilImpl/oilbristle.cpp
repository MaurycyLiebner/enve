#include "oilbristle.h"

#include <math.h>

#include "exceptions.h"

OilBristle::OilBristle(const QPointF& position, float length) {
	// Check that the input makes sense
    if(length <= 0) RuntimeThrow("There bristle length should be higher than zero.");

	// Fill the positions and lengths containers
	unsigned int nElements = round(sqrt(2 * length));
    positions = vector<QPointF>(nElements + 1, position);

	for (unsigned int i = 0; i < nElements; ++i) {
		lengths.push_back(nElements - i);
	}
}

void OilBristle::updatePosition(const QPointF& newPosition) {
	// Set the first element head position
	positions[0] = newPosition;

	// Set the elements tail positions
	for (unsigned int i = 0, nElements = getNElements(); i < nElements; ++i) {
        const QPointF& headPos = positions[i];
        QPointF& tailPos = positions[i + 1];
        float length = lengths[i];
        float ang = std::atan2(headPos.y() - tailPos.y(),
                               headPos.x() - tailPos.x());
        tailPos.setX(headPos.x() - length * cos(ang));
        tailPos.setY(headPos.y() - length * sin(ang));
	}
}

void OilBristle::setElementsPositions(const QPointF& newPosition) {
    for (QPointF& pos : positions) {
		pos = newPosition;
	}
}

void OilBristle::setElementsLengths(const vector<float>& newLengths) {
	// Check that the input makes sense
    if(newLengths.size() != getNElements()) {
        RuntimeThrow("The newLengths vector should contain as many values as elements in the bristle");
	}

	lengths = newLengths;
}

void OilBristle::paint(const QColor& color, float thickness) const {
	// Set the stroke color
	ofSetColor(color);

	// Paint the bristle elements
	unsigned int nElements = getNElements();
	float deltaThickness = thickness / nElements;

	for (unsigned int i = 0; i < nElements; ++i) {
		ofSetLineWidth(thickness - i * deltaThickness);
        ofDrawLine(positions[i].x(), positions[i].y(), 0,
                   positions[i + 1].x(), positions[i + 1].y(), 0);
	}
}

unsigned int OilBristle::getNElements() const {
	return lengths.size();
}
