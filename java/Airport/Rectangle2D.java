package cmsc420_s21;

// YOU SHOULD NOT MODIFY THIS FILE. If you want to add additional functionality,
// create a new class/file (e.g., MyRectangle2D.java) and modify that.

/**
 * A simple 2-dimensional rectangle.
 */

public class Rectangle2D {
	final static int DIM = 2; // spatial dimension
	Point2D low; // lower-left corner
	Point2D high; // upper-right corner

	/**
	 * Construct from two corner points.
	 * 
	 * @param c1 One corner of the rectangle
	 * @param c2 The opposite corner
	 */
	public Rectangle2D(Point2D c1, Point2D c2) {
		this.low  = new Point2D(Math.min(c1.getX(), c2.getX()), Math.min(c1.getY(), c2.getY()));
		this.high = new Point2D(Math.max(c1.getX(), c2.getX()), Math.max(c1.getY(), c2.getY()));
	}

	/**
	 * Copy constructor
	 * 
	 * @param r The source rectangle
	 */
	public Rectangle2D(Rectangle2D r) {
		low = new Point2D(r.low);
		high = new Point2D(r.high);
	}

	public String toString() {
		return "[" + low + "," + high + "]";
	}

	/**
	 * Getters
	 */

	public Point2D getLow() {
		return low;
	}

	public Point2D getHigh() {
		return high;
	}

	/**
	 * Get width along dimension i.
	 * 
	 * @param i The dimension.
	 * @return The absolute width along this direction
	 */
	public double getWidth(int i) {
		return high.get(i) - low.get(i);
	}

	/**
	 * Get center point.
	 * 
	 * @return The center point of the rectangle
	 */
	public Point2D getCenter() {
		return new Point2D((low.getX() + high.getX())/2, (low.getY() + high.getY())/2);
	}

	/**
	 * Check whether we contain a given point.
	 * 
	 * @param q The point.
	 * @return True if q lies within this (closed) rectangle.
	 */
	public boolean contains(Point2D q) {
		for (int i = 0; i < DIM; i++) {
			if (q.get(i) < low.get(i) || q.get(i) > high.get(i))
				return false;
		}
		return true;
	}

	/**
	 * Check whether we contain another rectangle.
	 * 
	 * @param c The other rectangle.
	 * @return True if c lies within this (closed) rectangle.
	 */
	public boolean contains(Rectangle2D c) {
		for (int i = 0; i < DIM; i++) {
			if (c.low.get(i) < low.get(i) || c.high.get(i) > high.get(i))
				return false;
		}
		return true;
	}

	/**
	 * Check whether we are disjoint from another rectangle. Rectangles are closed,
	 * so if their boundaries overlap, they are not disjoint.
	 * 
	 * @param c The other rectangle.
	 * @return True if we are completely disjoint from c.
	 */
	public boolean disjointFrom(Rectangle2D c) {
		for (int i = 0; i < DIM; i++) {
			if (c.high.get(i) < low.get(i) || c.low.get(i) > high.get(i))
				return true;
		}
		return false;
	}

	/**
	 * Compute the squared Euclidean distance to a point. Returns zero if the point
	 * is contained within this rectangle.
	 * 
	 * @param pt The point.
	 * @return Squared distance from pt to its closest point of this rectangle.
	 */
	public double distanceSq(Point2D pt) {
		double sum = 0; // sum of squared coordinate distances
		for (int i = 0; i < DIM; i++) {
			double coord = pt.get(i); // q's i-th coordinate
			double lc = low.get(i); // low's i-th coordinate
			double hc = high.get(i); // high's i-th coordinate
			if (coord < lc) { // to the left of the rectangle
				sum += Math.pow((lc - coord), 2);
			} else if (coord > hc) {
				sum += Math.pow((coord - hc), 2);
			}
		}
		return sum;
	}

	/**
	 * Compute a minimum rectangle that encloses the union of two
	 * rectangles. To do this, we start with one rectangle and expand
	 * it to include the opposing corners of the other.
	 * 
	 * @param r1 First rectangle.
	 * @param r2 Second rectangle.
	 * @return The enclosing rectangle.
	 */
	public static Rectangle2D union(Rectangle2D r1, Rectangle2D r2) {
		Rectangle2D result = new Rectangle2D(r1);
		result.add(r2.low);
		result.add(r2.high);
		return result;
	}

	/**
	 * Compute the left part of a rectangle that is split by a 
	 * line orthogonal to the cutting dimension and passing through
	 * a given point. It is assumed that s lies within the rectangle,
	 * or bizarre things can happen. (No error checking!)
	 * 
	 * @param cutDim The cutting dimension (0 = x, 1 = y).
	 * @return s The point about which to split.
	 */
	public Rectangle2D leftPart(int cutDim, Point2D s) {
		Rectangle2D result = new Rectangle2D(this);
		result.high.set(cutDim, s.get(cutDim));
		return result;
	}

	/**
	 * Compute the right part of a rectangle that is split by a 
	 * line orthogonal to the cutting dimension and passing through
	 * a given point. It is assumed that s lies within the rectangle,
	 * or bizarre things can happen. (No error checking!)
	 * 
	 * @param cutDim The cutting dimension (0 = x, 1 = y).
	 * @return s The point about which to split.
	 */
	public Rectangle2D rightPart(int cutDim, Point2D s) {
		Rectangle2D result = new Rectangle2D(this);
		result.low.set(cutDim, s.get(cutDim));
		return result;
	}

	/**
	 * Adds a point to a rectangle, by expanding it to include the point.
	 * 
	 * @param pt The point to add.
	 */
	public void add(Point2D pt) {
		for (int i = 0; i < DIM; i++) {
			double coord = pt.get(i);
			if (coord < low.get(i))
				low.set(i, coord);
			if (coord > high.get(i))
				high.set(i, coord);
		}
	}

}
