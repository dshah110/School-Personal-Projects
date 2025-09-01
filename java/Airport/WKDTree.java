/**
 * This solution was sourced from course materials.
 */


package cmsc420_s21;

import java.util.ArrayList;

/**
 * Wrapped kd-Tree
 *
 * A wrapped kd-tree is an extended version of a kd-tree, where each internal
 * node stores an axis-aligned bounding box (or "wrapper") for the points stored
 * in this node's subtree.
 *
 * Generic Elements: The tree is parameterized by a Point, which implements the
 * LabeledPoint2D interface.
 * 
 * In addition to its left and right children, each internal node stores a
 * cutting dimension (either 0 for x or 1 for y), a cutting value, and an
 * axis-aligned bounding box for the points of this subtree. Each external node
 * stores just a single point.
 */

public class WKDTree<LPoint extends LabeledPoint2D> {

	private final boolean DEBUG = false; // produce extra debugging output
	private final boolean VALIDATE = false; // validate structure integrity

	// -----------------------------------------------------------------
	// Nodes
	// -----------------------------------------------------------------

	/**
	 * A node of the tree. This is an abstract object, which is extended for
	 * internal and external nodes.
	 */
	private abstract class Node { // generic node type

		// -----------------------------------------------------------------
		// Standard dictionary helpers
		// -----------------------------------------------------------------

		abstract LPoint find(Point2D pt); // find point in subtree

		abstract Node insert(LPoint pt) throws Exception; // insert point into subtree

		abstract Node delete(Point2D pt) throws Exception; // delete point from subtree

		abstract ArrayList<String> getPreorderList(); // list entries in preorder

		// -----------------------------------------------------------------
		// Queries
		// -----------------------------------------------------------------

		abstract LPoint getMinMax(int dim, int sign); // get min/max along dim

		abstract LPoint findSmallLargeXY(int dim, int sign, double x, LPoint best); // find point with smaller/larger X/Y

		abstract ArrayList<LPoint> circularRange(Point2D center, double sqRadius); // circular range query

		// -----------------------------------------------------------------
		// Other utilities
		// -----------------------------------------------------------------

		abstract Rectangle2D getWrapper(); // get the wrapper

		abstract ExternalNode findExt(Point2D pt); // find external node containing point

		abstract String debugPrint(String prefix); // print for debugging

		abstract public String toString(); // string representation

		boolean onLeft(LPoint pt, int cutDim, double cutVal) { // in the left subtree?
			return pt.get(cutDim) < cutVal;
		}

		boolean onLeft(Point2D pt, int cutDim, double cutVal) { // in the left subtree?
			return pt.get(cutDim) < cutVal;
		}

		abstract void validate(Rectangle2D bounding); // validate tree's structure (for debugging)

	}

	// -----------------------------------------------------------------
	// Internal node
	// -----------------------------------------------------------------

	private class InternalNode extends Node {

		int cutDim; // the cutting dimension (0 = x, 1 = y)
		double cutVal; // the cutting value
		Rectangle2D wrapper; // bounding box
		Node left, right; // children

		/**
		 * Constructor
		 */
		InternalNode(int cutDim, double cutVal, Node left, Node right) {
			this.cutDim = cutDim;
			this.cutVal = cutVal;
			this.left = left;
			this.right = right;
			this.wrapper = Rectangle2D.union(left.getWrapper(), right.getWrapper());
		}

		// -----------------------------------------------------------------
		// Standard dictionary helpers
		// -----------------------------------------------------------------

		/**
		 * Find point in this subtree
		 */
		LPoint find(Point2D pt) {
			if (!wrapper.contains(pt))
				return null;
			else if (onLeft(pt, cutDim, cutVal))
				return left.find(pt);
			else
				return right.find(pt);
		}

		/**
		 * Insert into this subtree
		 */
		Node insert(LPoint pt) throws Exception {
			if (onLeft(pt, cutDim, cutVal)) { // insert on appropriate side
				left = left.insert(pt);
			} else {
				right = right.insert(pt);
			}
			wrapper = Rectangle2D.union(left.getWrapper(), right.getWrapper()); // update wrapper
			return this;
		}

		/**
		 * Delete a point from a subtree
		 */
		Node delete(Point2D pt) throws Exception {
			if (onLeft(pt, cutDim, cutVal)) {
				left = left.delete(pt);
				if (left == null) {
					return right; // subtree gone, return sibling
				}
			} else {
				right = right.delete(pt); // update this node's information
				if (right == null) {
					return left; // subtree gone, return sibling
				}
			}
			wrapper = Rectangle2D.union(left.getWrapper(), right.getWrapper()); // update wrapper
			return this;
		}

		/**
		 * Get a list of the nodes in preorder.
		 *
		 * Adds current key in parentheses, followed by left and right subtrees.
		 */
		ArrayList<String> getPreorderList() {
			ArrayList<String> list = new ArrayList<String>();
			list.add(toString()); // add this node
			list.addAll(left.getPreorderList()); // add left
			list.addAll(right.getPreorderList()); // add right
			return list;
		}

		// -----------------------------------------------------------------
		// Queries
		// -----------------------------------------------------------------

		/**
		 * Get min/max along the given dim. (sign = -1/+1 for min/max)
		 */
		LPoint getMinMax(int dim, int sign) {
			if (cutDim == dim) { // need only visit one side
				if (sign < 0)
					return left.getMinMax(dim, sign);
				else
					return right.getMinMax(dim, sign);
			} else {
				LPoint pt1 = left.getMinMax(dim, sign); // get min/max from each subtree
				LPoint pt2 = right.getMinMax(dim, sign);
				LPoint result = minMax(dim, sign, pt1, pt2); // get the better of two
				if (result == null) { // tied for best?
					result = minMax(1 - dim, sign, pt1, pt2); // use the other coordinate
				}
				return result;
			}
		}

		/**
		 * Selects between pt1 or pt2 depending on which has smaller dim coordinate (if
		 * sign = -1) or larger dim coordinate (if sign = +1).
		 */
		LPoint minMax(int dim, int sign, LPoint pt1, LPoint pt2) {
			double x1 = pt1.get(dim); // discriminating coordinates
			double x2 = pt2.get(dim);
			double disc = (x2 - x1) * sign; // discriminant test
			if (disc == 0) // equal?
				return null;
			else if (disc < 0) // pt1 is better
				return pt1;
			else // pt2 is better
				return pt2;
		}

		/**
		 * Returns point with smaller/larger X/Y
		 */
		LPoint findSmallLargeXY(int dim, int sign, double x, LPoint best) {
			double lowCoord = wrapper.getLow().get(dim);
			double highCoord = wrapper.getHigh().get(dim);
			double primeCoord = (sign < 0 ? lowCoord : highCoord);
			double secondCoord = (sign < 0 ? highCoord : lowCoord);
			double extreme = (sign < 0 ? Float.NEGATIVE_INFINITY : Float.POSITIVE_INFINITY);
			if (sign * (primeCoord - x) > 0) { // possibly relevant?
				double bestX = (best == null ? extreme : best.get(dim));
				if (sign * (secondCoord - bestX) <= 0) { // contender for best?
					best = right.findSmallLargeXY(dim, sign, x, best); // search the children
					best = left.findSmallLargeXY(dim, sign, x, best);
				}
			}
			return best; // return the current best
		}

		/**
		 * Circular range reporting for a disk of a given squared radius about a given
		 * center point.
		 */
		ArrayList<LPoint> circularRange(Point2D center, double sqRadius) {
			ArrayList<LPoint> list = new ArrayList<LPoint>();
			if (wrapper.distanceSq(center) <= sqRadius) {
				list.addAll(left.circularRange(center, sqRadius));
				list.addAll(right.circularRange(center, sqRadius));
			}
			return list;
		}

		// -----------------------------------------------------------------
		// Other utilities
		// -----------------------------------------------------------------

		/**
		 * Get the wrapper
		 */
		Rectangle2D getWrapper() {
			return wrapper;
		}

		/**
		 * Find external node containing point
		 */
		ExternalNode findExt(Point2D pt) {
			return onLeft(pt, cutDim, cutVal) ? left.findExt(pt) : right.findExt(pt);
		}

		/**
		 * String representation.
		 */
		public String toString() {
			String cutIndic = (cutDim == 0 ? "x=" : "y=");
			return "(" + cutIndic + cutVal + "): " + wrapper;
		}

		/**
		 * Debug print subtree.
		 *
		 * This prints the subtree for debugging.
		 */
		String debugPrint(String prefix) {
			return left.debugPrint(prefix + "| ") + System.lineSeparator() + prefix + toString() // + ": " + level
					+ System.lineSeparator() + right.debugPrint(prefix + "| ");
		}

		void validate(Rectangle2D bounding) {
		}
	}

	// -----------------------------------------------------------------
	// External node
	// -----------------------------------------------------------------

	private class ExternalNode extends Node {

		private LPoint thisPt; // the associated point

		/**
		 * Constructor from a point.
		 */
		ExternalNode(LPoint pt) {
			thisPt = pt;
		}

		// -----------------------------------------------------------------
		// Standard dictionary helpers
		// -----------------------------------------------------------------

		/**
		 * Find point in external node.
		 */
		LPoint find(Point2D pt) {
			if (thisPt.getPoint2D().equals(pt))
				return thisPt;
			else
				return null;
		}

		/**
		 * Insert a point into this node.
		 */
		Node insert(LPoint pt) throws Exception {
			if (thisPt.getPoint2D().equals(pt.getPoint2D())) { // same point exists?
				throw new Exception("Insertion of point with duplicate coordinates");
			} else {
				Rectangle2D wrapper = new Rectangle2D(thisPt.getPoint2D(), pt.getPoint2D());
				int cutDim = (wrapper.getWidth(0) >= wrapper.getWidth(1) ? 0 : 1); // cutting dimension is longer side
				double cutVal = (pt.get(cutDim) + thisPt.get(cutDim)) / 2; // cutting value is midpoint
				ExternalNode q = new ExternalNode(pt); // new external node
				if (onLeft(pt, cutDim, cutVal)) { // insert on left side
					return new InternalNode(cutDim, cutVal, q, this);
				} else { // insert on right side
					return new InternalNode(cutDim, cutVal, this, q);
				}
			}

		}

		/**
		 * Delete a point from this node.
		 */
		Node delete(Point2D pt) throws Exception {
			if (thisPt.getPoint2D().equals(pt)) { // found it?
				return null; // just unlink the node
			} else {
				throw new Exception("Deletion of nonexistent point");
			}
		}

		/**
		 * Add entry to the list.
		 * 
		 * @param list The list into which items are added
		 */
		ArrayList<String> getPreorderList() {
			ArrayList<String> list = new ArrayList<String>();
			list.add(toString()); // add this node
			return list;

		}

		// -----------------------------------------------------------------
		// Queries
		// -----------------------------------------------------------------

		/**
		 * Get min/max along the given dim. (sign = -1/+1 for min/max)
		 */
		LPoint getMinMax(int dim, int sign) {
			return thisPt;
		}

		/**
		 * Returns the point with strictly smaller x coordinate.
		 */
		LPoint findSmallLargeXY(int dim, int sign, double x, LPoint best) {
			double thisX = thisPt.get(dim);
			if (sign * (thisX - x) > 0) { // possibly relevant?
				double extreme = (sign < 0 ? Float.NEGATIVE_INFINITY : Float.POSITIVE_INFINITY);
				double bestX = (best == null ? extreme : best.get(dim));
				if (sign * (thisX - bestX) < 0) { // better than current best?
					best = thisPt;
				} else if (thisX == bestX) { // tied with best?
					double thisY = thisPt.get(1 - dim); // select other coord
					double bestY = best.get(1 - dim);
					if (sign * (thisY - bestY) < 0)
						best = thisPt;
				}
			}
			return best;
		}

		/**
		 * Circular range reporting for a disk of a given squared radius about a given
		 * center point.
		 */
		ArrayList<LPoint> circularRange(Point2D center, double sqRadius) {
			ArrayList<LPoint> list = new ArrayList<LPoint>();
			if (center.distanceSq(thisPt.getPoint2D()) <= sqRadius)
				list.add(thisPt);
			return list;
		}

		// -----------------------------------------------------------------
		// Other utilities
		// -----------------------------------------------------------------

		/**
		 * Get the bounding box (a trivial rectangle containing just the point).
		 */
		Rectangle2D getWrapper() {
			return new Rectangle2D(thisPt.getPoint2D(), thisPt.getPoint2D());
		}

		/**
		 * Find the external node containing a point.
		 */
		ExternalNode findExt(Point2D pt) {
			return this;
		}

		/**
		 * String representation.
		 */
		public String toString() {
			return "[" + thisPt.toString() + "]";
		}

		/**
		 * Debug print subtree.
		 * 
		 * @param prefix String indentation
		 */
		String debugPrint(String prefix) {
			return prefix + toString();
		}

		void validate(Rectangle2D bounding) {
		}
	}

	// -----------------------------------------------------------------
	// Private data
	// -----------------------------------------------------------------

	private Node root; // root of the tree
	private int nPoints; // number of points in the tree

	// -----------------------------------------------------------------
	// Public members
	// -----------------------------------------------------------------

	/**
	 * Creates an empty tree.
	 */
	public WKDTree() {
		root = null;
		nPoints = 0;
	}

	/**
	 * Number of entries in the dictionary.
	 *
	 * @return Number of entries in the dictionary
	 */
	public int size() {
		return nPoints;
	}

	/**
	 * Find an point in the tree. Note that the point being deleted does not need to
	 * match fully. It suffices that it has enough information to satisfy the
	 * comparator.
	 *
	 * @param pt The item being sought (only the relevant members are needed)
	 * @return A reference to the element where found or null if not found
	 */
	public LPoint find(Point2D pt) {
		if (root == null) {
			return null;
		} else {
			return root.find(pt);
		}
	}

	/**
	 * Insert a point
	 *
	 * @param point The point to be inserted
	 * @throws Exception if point with same coordinates exists in the tree
	 */
	public void insert(LPoint pt) throws Exception {
		if (DEBUG) {
			System.out.println(System.lineSeparator() + "wkd-tree: Inserting " + pt);
		}
		if (root == null) {
			root = new ExternalNode(pt);
		} else {
			root = root.insert(pt);
		}
		nPoints += 1; // one more point
		if (DEBUG) {
			System.out.println(
					System.lineSeparator() + "wkd-tree: After insertion" + System.lineSeparator() + debugPrint("  "));
		}
		if (VALIDATE)
			validate();
	}

	/**
	 * Delete a point. Note that the point being deleted does not need to match
	 * fully. It suffices that it has enough information to satisfy the comparator.
	 *
	 * @param point The point to be deleted
	 * @throws Exception if point with same coordinates exists in the tree
	 */
	public void delete(Point2D pt) throws Exception {
		if (DEBUG) {
			System.out.println(System.lineSeparator() + "wkd-tree: Deleting " + pt);
		}
		if (root == null) {
			throw new Exception("Deletion of nonexistent point");
		} else {
			root = root.delete(pt);
		}
		nPoints -= 1; // one fewer point
		if (DEBUG) {
			System.out.println(
					System.lineSeparator() + "wkd-tree: After deletion" + System.lineSeparator() + debugPrint("  "));
		}
	}

	/**
	 * Get a list of entries in preorder
	 *
	 * @return ArrayList of string encoded items in preorder
	 */
	public ArrayList<String> getPreorderList() {
		if (root == null) { // empty?
			return new ArrayList<String>(); // return an empty list
		} else {
			return root.getPreorderList();
		}
	}

	/**
	 * Remove all items, resulting in an empty tree
	 */
	public void clear() {
		root = null;
		nPoints = 0;
	}

	/**
	 * Get point with min/max x/y coordinate. Ties are broken lexicographically.
	 *
	 * @return A reference to the associated value, or null if tree empty
	 */
	public LPoint getMinX() {
		if (root == null)
			return null;
		else
			return root.getMinMax(0, -1);
	}

	public LPoint getMaxX() {
		if (root == null)
			return null;
		else
			return root.getMinMax(0, +1);
	}

	public LPoint getMinY() {
		if (root == null)
			return null;
		else
			return root.getMinMax(1, -1);
	}

	public LPoint getMaxY() {
		if (root == null)
			return null;
		else
			return root.getMinMax(1, +1);
	}

	/**
	 * Get point with strictly smaller/larger x/y coordinate. Ties are broken
	 * lexicographically.
	 *
	 * @param x The threshold value
	 * @return A reference to the associated value, or null if tree empty
	 */
	public LPoint findSmallerX(double x) {
		LPoint best = null;
		if (root == null)
			return null;
		else
			return root.findSmallLargeXY(0, -1, x, best);
	}
	
	public LPoint findLargerX(double x) {
		LPoint best = null;
		if (root == null)
			return null;
		else
			return root.findSmallLargeXY(0, +1, x, best);
	}
	
	public LPoint findSmallerY(double y) {
		LPoint best = null;
		if (root == null)
			return null;
		else
			return root.findSmallLargeXY(1, -1, y, best);
	}
	
	public LPoint findLargerY(double y) {
		LPoint best = null;
		if (root == null)
			return null;
		else
			return root.findSmallLargeXY(1, +1, y, best);
	}

	/**
	 * Find the nearest neighbor of a query point.
	 * 
	 * @param q The query point.
	 * @return The nearest neighbor to q or null if the tree is empty.
	 */
	public ArrayList<LPoint> circularRange(Point2D center, double sqRadius) {
		if (root == null) {
			return new ArrayList<LPoint>(); // return an empty list
		} else {
			return root.circularRange(center, sqRadius);
		}
	}

	// -----------------------------------------------------------------
	// Debugging utilities
	// -----------------------------------------------------------------

	void validate() {
		if (root == null) {
			if (nPoints != 0)
				System.err.println("Validation error: Incorrect number of points (should be zero)");
		} else {
			root.validate(null); // validate the structure
			ArrayList<String> list = root.getPreorderList();
			if (list.size() != 2 * nPoints - 1) { // number of nodes should be 2 * n - 1
				System.err.println("Validation error: Incorrect number of entries");
			}
		}
	}

	/**
	 * Print the tree for debugging purposes
	 * 
	 * @param prefix A string prefix that precedes each line.
	 * @return A string representing the tree
	 */
	String debugPrint(String prefix) {
		if (root != null)
			return root.debugPrint(prefix);
		else
			return new String();
	}
	
	// -----------------------------------------------------------------
	// New for Programming Assignment 3 - Fixed-radius nearest neighbor
	// -----------------------------------------------------------------
	
	public LPoint fixedRadNN(Point2D center, double sqRadius) { /* ... */ 
		ArrayList<LPoint> list = circularRange(center, sqRadius);
		for(LPoint p : list) {
			if(p.getPoint2D().distanceSq(center) >= sqRadius || p.getPoint2D().distanceSq(center) <= 0) {
				list.remove(p);
			}
		}
		if(list.isEmpty() == true) {
			return null;
		}
		else {
			double minx = center.getX() + sqRadius;
			double miny = center.getY() + sqRadius;
			double sd = sqRadius;
			for(LPoint p : list) {
				if(p.getPoint2D().distanceSq(center) < sd) {
					sd = p.getPoint2D().distanceSq(center);
				}
			}
			for(LPoint p : list) {
				if(p.getPoint2D().distanceSq(center) > sd) {
					list.remove(p);
				}
			}
			for(LPoint p : list) {
				if(p.getX() < minx) {
					minx = p.getX();
				}
			}
			for(LPoint p : list) {
				if(p.getX() > minx) {
					list.remove(p);
				}
			}
			for(LPoint p : list) {
				if(p.getY() < minx) {
					miny = p.getY();
				}
			}
			for(LPoint p : list) {
				if(p.getY() > miny) {
					list.remove(p);
				}
			}
			return list.get(0);
			
			//list is now a list of all points that are the closest to center.
			
		}
	}

}
