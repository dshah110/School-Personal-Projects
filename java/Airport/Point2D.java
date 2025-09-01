package cmsc420_s21;

// YOU SHOULD NOT MODIFY THIS FILE. If you want to add additional functionality,
// create a new class/file (e.g., MyPoint2D.java) and modify that.

/**
 * A 2-dimensional point. We represent a point as a 2-element array of type
 * double.
 */

public class Point2D {
	final static int DIM = 2; // spatial dimension
	double[] coord; // coordinates

	/**
	 * Default constructor.
	 * 
	 */
	public Point2D() {
		coord = new double[DIM];
		for (int i = 0; i < DIM; i++)
			coord[i] = 0;
	}

	/**
	 * Construct from coordinates.
	 * 
	 * @param coord The array of coordinates.
	 */
	public Point2D(double x, double y) {
		coord = new double[DIM];
		coord[0] = x;
		coord[1] = y;
	}

	/**
	 * Construct from a 2-element coordinate array.
	 * 
	 * @param coord The array of coordinates.
	 */
	public Point2D(double[] coord) {
		assert (coord.length == DIM);
		this.coord = new double[DIM];
		for (int i = 0; i < DIM; i++)
			this.coord[i] = coord[i];
	}

	/**
	 * Copy constructor.
	 * 
	 * @param p The point to copy.
	 */
	public Point2D(Point2D p) {
		coord = new double[DIM];
		for (int i = 0; i < DIM; i++)
			coord[i] = p.get(i);
	}

	/**
	 * Get the dimension.
	 * 
	 * @return The dimension
	 */
	public static int getDim() {
		return DIM;
	}

	/**
	 * Get the i-th coordinate (i=0 for x, i=1 for y).
	 * 
	 * @param i The coordinate index 0 or 1.
	 * @return The i-th coordinate of the point.
	 */
	public double get(int i) {
		return coord[i];
	}

	/**
	 * Set the i-th coordinate (i=0 for x, i=1 for y).
	 * 
	 * @param i The coordinate index 0 or 1.
	 * @param x The new coordinate value.
	 */
	public void set(int i, double x) {
		coord[i] = x;
	}

	/**
	 * Set both coordinates.
	 * 
	 * @param x The new x-coordinate.
	 * @param y The new y-coordinate.
	 */
	public void setLocation(double x, double y) {
		coord[0] = x;
		coord[1] = y;
	}

	/**
	 * Get the x-coordinate.
	 * 
	 * @return The x-coordinate of the point.
	 */
	public double getX() {
		return coord[0];
	}

	/**
	 * Get the y-coordinate.
	 * 
	 * @return The y-coordinate of the point.
	 */
	public double getY() {
		return coord[1];
	}

	/**
	 * Equality test.
	 * 
	 * @param pt The other point
	 * @return True if equal
	 */
	public boolean equals(Point2D pt) {
		for (int i = 0; i < DIM; i++) {
			if (pt.coord[i] != coord[i]) return false;
		}
		return true;
	}

	/**
	 * Euclidean distance to point.
	 * 
	 * @param pt The other point
	 * @return The distance
	 */
	public double distance(Point2D pt) {
		double sum = 0;
		for (int i = 0; i < DIM; i++) {
			sum += Math.pow(pt.coord[i] - coord[i], 2);
		}
		return (double) Math.sqrt(sum);
	}

	/**
	 * Squared Euclidean distance to point.
	 * 
	 * @param pt The other point
	 * @return The squared distance
	 */
	public double distanceSq(Point2D pt) {
		double sum = 0;
		for (int i = 0; i < DIM; i++) {
			sum += Math.pow(pt.coord[i] - coord[i], 2);
		}
		return sum;
	}

	/**
	 * String representation.
	 * 
	 * @return String representation of the point.
	 */
	public String toString() {
		return "(" + coord[0] + "," + coord[1] + ")";
	}

}
