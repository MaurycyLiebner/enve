#pragma once

#include <vector>
using std::vector;

#include <QPointF>
#include <QColor>

/**
 * @brief Class that simulates the movement of a bristle
 *
 * @author Javier Graciá Carpio
 */
class OilBristle {
public:

	/**
	 * @brief Constructor
	 *
	 * @param position the bristle position
	 * @param length the bristle total length
	 */
    OilBristle(const QPointF& position = QPointF(), float length = 10);

	/**
	 * @brief Updates the bristle position
	 *
	 * @param newPosition the new bristle position
	 */
    void updatePosition(const QPointF& newPosition);

	/**
	 * @brief Sets the bristle elements positions
	 *
	 * @param newPosition the new bristle elements position
	 */
    void setElementsPositions(const QPointF& newPosition);

	/**
	 * @brief Sets the bristle elements lengths
	 *
	 * @param newLengths the new bristle elements lengths
	 */
    void setElementsLengths(const vector<float>& newLengths);

	/**
	 * @brief Paints the bristle
	 *
	 * @param color the color to use
	 * @param thickness the thickness of the first bristle element
	 */
    void paint(const QColor& color, float thickness) const;

	/**
	 * @brief Returns the number of bristle elements
	 *
	 * @return the number of bristle elements
	 */
    unsigned int getNElements() const;

protected:

	/**
	 * @brief The bristle elements positions
	 */
    vector<QPointF> positions;

	/**
	 * @brief The bristle elements lengths
	 */
    vector<float> lengths;
};
