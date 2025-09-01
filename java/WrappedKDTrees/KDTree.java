package cmsc420_s21; // Don't delete this or your file won't pass the autograder

import java.util.ArrayList;

/**
 * AAX - An extended AA Tree.
 *
 * Generic Elements: The tree is parameterized by two types, Key and Value. The
 * first is used for searching and the second is simply stored. Key-value pairs
 * are stored at the leaves and keys are stored in the internal nodes.
 */

public class AAXTree<Key extends Comparable<Key>, Value> {

	// -----------------------------------------------------------------
	// Activate/deactivate debug output and structure validation
	// -----------------------------------------------------------------
	private final boolean DEBUG = false; // standard mode
	// private final boolean DEBUG = true; // generate additional debugging output
	
	private final boolean VALIDATE = false; // no validation
	// private final boolean VALIDATE = true; // validate the structure after each modification

	// -----------------------------------------------------------------
	// Nodes
	// -----------------------------------------------------------------

	/**
	 * A node of the tree. This is an abstract object, which is extended for
	 * internal and external nodes.
	 */
	private abstract class Node { // generic node type

		Node() {
		} // basic constructor

		// -----------------------------------------------------------------
		// Standard dictionary helpers
		// -----------------------------------------------------------------

		abstract Value find(Key x); // find key in subtree

		abstract Node insert(Key x, Value v) throws Exception; // insert key-value into subtree

		abstract Node delete(Key x) throws Exception; // delete key from subtree

		abstract ArrayList<String> getPreorderList(); // list entries in preorder

		// -----------------------------------------------------------------
		// Enhanced operation helpers
		// -----------------------------------------------------------------

		abstract Value getMin(); // get value of minimum key in subtree

		abstract Value getMax(); // get value of maximum key in subtree

		abstract Value findSmaller(Key x); // find next that is strictly smaller

		abstract Value findLarger(Key x); // find next that is strictly larger

		abstract Node removeMin(); // remove entry with the smallest key

		abstract Node removeMax(); // remove entry with the largest key

		// -----------------------------------------------------------------
		// Accessors
		// -----------------------------------------------------------------

		abstract int getLevel(); // get left child (null if external)

		abstract void setLevel(int level); // set level (ignored if external)

		abstract void incrementLevel(); // increment level (ignored if external)

		abstract Node getLeft(); // get left child (null if external)

		abstract Node getRight(); // get right child (null if external)

		abstract void setLeft(Node p); // set left child (ignored if external)

		abstract void setRight(Node p); // set right child (ignored if external)

		// -----------------------------------------------------------------
		// Rebalancing utilities
		// -----------------------------------------------------------------

		abstract Node skew(); // right skew a node (ignored if external)

		abstract Node split(); // split a node (ignored if external)

		abstract void updateLevel(); // update level (ignored if external)

		abstract Node fixAfterDelete(); // fix structures after deletion (ignored if external)

		// -----------------------------------------------------------------
		// Other utilities
		// -----------------------------------------------------------------

		abstract String debugPrint(String prefix); // print (for debugging)

		abstract public String toString(); // print node contents

		abstract void validate(Key minKey, Key maxKey); // validate tree's structure (for debugging)
	}

	// -----------------------------------------------------------------
	// Internal node
	// -----------------------------------------------------------------

	private class InternalNode extends Node {
		Key key; // splitting key
		int level; // level in the tree
		Node left; // children
		Node right;

		/**
		 * Basic constructor.
		 */
		InternalNode(Key x, int level, Node left, Node right) {
			this.key = x;
			this.level = level;
			this.left = left;
			this.right = right;
		}

		// -----------------------------------------------------------------
		// Accessors
		// -----------------------------------------------------------------

		int getLevel() {
			return level;
		}

		void setLevel(int level) {
			this.level = level;
		}

		void incrementLevel() {
			this.level += 1;
		}

		Node getLeft() {
			return left;
		}

		Node getRight() {
			return right;
		}

		void setLeft(Node p) {
			left = p;
		}

		void setRight(Node p) {
			right = p;
		}

		// -----------------------------------------------------------------
		// Rebalancing utilities
		// -----------------------------------------------------------------

		/**
		 * Right skew a node. If the left child has same level as us, perform a right
		 * rotation.
		 */
		Node skew() {
			if (DEBUG) System.out.print("...Invoking skew at " + toString());
			
			if (left.getLevel() == level) { // left child at same level as us?
				Node q = left;
				left = q.getRight(); // rotate right
				q.setRight(this);
				if (DEBUG) System.out.println(" --> Current subtree after rotation:" + System.lineSeparator() + q.debugPrint("......"));
				return q; // return new top node
			} else {
				if (DEBUG) System.out.println(" --> No rotation needed");
				return this; // no change needed
			}
		}

		/**
		 * Split a node. If the right-right grandchild is at the same level, promote our
		 * right child to the next higher level.
		 */
		Node split() {
			if (DEBUG) System.out.print("...Invoking split at " + toString());
			if (right.getRight().getLevel() == level) { // right-right grandchild same level?
				Node q = right;
				right = q.getLeft(); // rotate left
				q.setLeft(this);
				q.incrementLevel(); // promote our right child
				if (DEBUG) System.out.println(" --> Current subtree after rotation:" + System.lineSeparator() + q.debugPrint("......"));
				return q; // return new top node
			} else {
				if (DEBUG) System.out.println(" --> No rotation needed");
				return this; // no change needed
			}
		}

		/**
		 * Update node's level from its children.
		 */
		void updateLevel() {
			if (DEBUG) System.out.print("...Invoking updateLevel at " + toString());
			int maxLevel = 1 + Math.min(left.getLevel(), right.getLevel()); // max allowed level
			if (level > maxLevel) { // our level is too high?
				level = maxLevel; // decrease our level
				if (DEBUG) System.out.print(" --> Level decreases to " + level); 
				if (right.getLevel() > maxLevel) { // also fix our right child
					right.setLevel(maxLevel);
					if (DEBUG) System.out.print(" --> Right child as well");
				}
				if (DEBUG) System.out.print(" --> Current subtree:" + System.lineSeparator() + debugPrint("......"));
			} else {
				if (DEBUG) System.out.print(" --> No level changes needed");
			}
			if (DEBUG) System.out.println();
		}

		/**
		 * Fix local structure after deletion. (See references on AA trees for an
		 * explanation of this combination of operations.)
		 */
		Node fixAfterDelete() {
			if (DEBUG) System.out.println("...Invoking fixAfterDelete at " + toString());
			updateLevel(); // update our level
			Node p = skew(); // skew us (p is new subtree root)
			p.setRight(p.getRight().skew()); // skew right child
			p.getRight().setRight(p.getRight().getRight().skew()); // skew right-right grandchild
			p = p.split(); // split top node
			p.setRight(p.getRight().split());
			return p;
		}

		// -----------------------------------------------------------------
		// Dictionary helper functions
		// -----------------------------------------------------------------

		/**
		 * Find a key.
		 */
		Value find(Key x) {
			if (x.compareTo(key) < 0) // x < key
				return left.find(x); // ... search left
			else // x >= key
				return right.find(x); // ... search right
		}

		/**
		 * Insert key-value pair. This just passes the call down to the appropriate
		 * child, and restores balance by applying skew and split.
		 */
		Node insert(Key x, Value v) throws Exception {
			if (x.compareTo(key) < 0) { // x < key
				left = left.insert(x, v); // ...insert left
			} else { // x >= key
				right = right.insert(x, v); // ...insert right
			}
			return skew().split(); // restore tree structure
		}

		/**
		 * Delete a key. We invoke the function on the appropriate child. If the node
		 * disappears (meaning it was an external node), we return a pointer to the
		 * other child (which must also be an external node).
		 */
		Node delete(Key x) throws Exception {
			if (x.compareTo(key) < 0) { // x < key
				left = left.delete(x); // ...delete from left
				if (left == null) { // left child vanished?
					return right; // return the other child
				}
			} else { // x >= key
				right = right.delete(x); // ...delete from right
				if (right == null) { // right child vanished?
					return left; // return the other child
				}
			}
			return fixAfterDelete(); // fix structure after deletion
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
		// Enhanced operation helpers
		// -----------------------------------------------------------------

		/**
		 * Get the value associated with the minimum key.
		 *
		 */
		Value getMin() {
			return left.getMin();
		}

		/**
		 * Get the value associated with the maximum key.
		 *
		 */
		Value getMax() {
			return right.getMax();
		}

		/**
		 * Find next value with strictly smaller key. We check
		 * the subtree where the node should belong. If we fail on
		 * the right side, we return the largest from the left.
		 */
		Value findSmaller(Key x) {
			if (x.compareTo(key) < 0) { 
				return left.findSmaller(x); // search left subtree
			} else { 
				Value v = right.findSmaller(x); // search right subtree
				return (v == null ? left.getMax() : v);
			}
		}

		/**
		 * Find next value with strictly larger key. We check
		 * the subtree where the node should belong. If we fail on
		 * the left side, we return the smallest from the right.
		 */
		Value findLarger(Key x) {
			if (x.compareTo(key) < 0) { 
				Value v = left.findLarger(x); // search left subtree
				return (v == null ? right.getMin() : v);
			} else { 
				return right.findLarger(x); // search right subtree
			}
		}

		/**
		 * Remove the entry with the smallest key.
		 */
		Node removeMin() {
			left = left.removeMin(); // ...remove from left subtree
			if (left == null) { // left child vanished?
				return right; // return the other child
			} else {
				return fixAfterDelete(); // fix structure after deletion
			}
		}

		/**
		 * Remove the entry with the largest key.
		 */
		Node removeMax() {
			right = right.removeMax(); // ...remove from right subtree
			if (right == null) { // right child vanished?
				return left; // return the other child
			} else {
				return fixAfterDelete(); // fix structure after deletion
			}
		}

		// -----------------------------------------------------------------
		// Other utilities
		// -----------------------------------------------------------------

		/**
		 * String representation.
		 */
		public String toString() {
			return "(" + key + ") " + level;
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

		/**
		 * Validate tree's structure.
		 */
		void validate(Key minKey, Key maxKey) {
			if (minKey != null && key.compareTo(minKey) < 0) { // key must be >= minKey
				System.err.println("Validation error: Key is too small");
			}
			if (maxKey != null && key.compareTo(maxKey) >= 0) { // key must be < maxKey
				System.err.println("Validation error: Key is too large");
			}
			if (left.getLevel() >= level) { // left child must be strictly smaller level
				System.err.println("Validation error: Left child level must be smaller");
			}
			if (right.getLevel() > level) { // right child level cannot be greater
				System.err.println("Validation error: Right child level cannot be greater");
			}
			if (right.getRight().getLevel() == level) { // right-right grandchild cannot be at same level
				System.err.println("Validation error: Right-right granchild level cannot be equal");
			}
			left.validate(minKey, key); // validate left child
			right.validate(key, maxKey); // validate right child
		}
	}

	// -----------------------------------------------------------------
	// External node
	// -----------------------------------------------------------------

	private class ExternalNode extends Node {
		Key key; // the key
		Value value; // the value

		/**
		 * Basic constructor.
		 */
		ExternalNode(Key key, Value value) {
			super();
			this.key = key;
			this.value = value;
		}

		// -----------------------------------------------------------------
		// Accessors - These are here to keep the compiler happy, but most
		// do nothing. getLeft() and getRight() return this to mimic the
		// sentinel node Nil in the AA tree.
		//
		// Note: updateLevel, fixAfterDelete, incrementLevel, setLevel,
		// and getLeft, setLeft are never called. We need them to keep the
		// compiler happy.
		// -----------------------------------------------------------------

		int getLevel() {
			return 0;
		}

		void incrementLevel() {
			return;
		}

		void setLevel(int level) {
			return;
		}

		Node getLeft() {
			return this;
		}

		Node getRight() {
			return this;
		}

		void setLeft(Node p) {
			return;
		}

		void setRight(Node p) {
			return;
		}

		// -----------------------------------------------------------------
		// Rebalancing utilities - Do nothing for external nodes
		// -----------------------------------------------------------------

		Node skew() {
			if (DEBUG) System.out.println("...Invoking skew at " + toString() + " --> No effect on external nodes");
			return this;
		}

		Node split() {
			if (DEBUG) System.out.println("...Invoking split at " + toString() + " --> No effect on external nodes");
			return this;
		}

		void updateLevel() {
			if (DEBUG) System.out.println("...Invoking updateLevel at " + toString() + " --> No effect on external nodes");
			return;
		}

		Node fixAfterDelete() {
			if (DEBUG) System.out.println("...Invoking fixAfterDelete at " + toString() + " --> No effect on external nodes");
			return this;
		}

		// -----------------------------------------------------------------
		// Dictionary helper functions
		// -----------------------------------------------------------------

		/**
		 * Find a key.
		 */
		Value find(Key x) {
			if (x.compareTo(key) == 0)
				return value;
			else
				return null;
		}

		/**
		 * Insert key-value pair. If the keys match, we throw a duplicate-key exception.
		 * Otherwise, we create a three-node combination with the two keys and a level-1
		 * internal node between them.
		 */
		Node insert(Key x, Value v) throws Exception {
			if (x.compareTo(key) == 0) {
				throw new Exception("Insertion of duplicate key");
			} else {
				ExternalNode q = new ExternalNode(x, v); // new external node
				if (x.compareTo(key) < 0) { // insert on left side (x must be smallest overall)
					return new InternalNode(key, 1, q, this);
				} else { // insert on right side (more typical)
					return new InternalNode(x, 1, this, q);
				}
			}
		}

		/**
		 * Delete a key. We simply unlink this node and return null;
		 */
		Node delete(Key x) throws Exception {
			if (x.compareTo(key) == 0) { // found it?
				return null; // just unlink the node
			} else {
				throw new Exception("Deletion of nonexistent key");
			}
		}

		/**
		 * Get preorder list. This just returns an encoding of the data in this node.
		 */
		ArrayList<String> getPreorderList() {
			ArrayList<String> list = new ArrayList<String>();
			list.add(toString()); // add this node
			return list;
		}

		// -----------------------------------------------------------------
		// Enhanced operations
		// -----------------------------------------------------------------

		/**
		 * Get the value associated with the minimum key.
		 */
		Value getMin() {
			return value;
		}

		/**
		 * Get the value associated with the maximum key.
		 */
		Value getMax() {
			return value;
		}

		/**
		 * Find next value with strictly smaller key.
		 */
		Value findSmaller(Key x) {
			if (x.compareTo(key) > 0)
				return value;
			else
				return null;
		}

		/**
		 * Find next value with strictly larger key.
		 */
		Value findLarger(Key x) {
			if (x.compareTo(key) < 0)
				return value;
			else
				return null;
		}

		/**
		 * Remove the entry with the smallest key.
		 */
		Node removeMin() {
			return null;
		}

		/**
		 * Remove the entry with the largest key.
		 */
		Node removeMax() {
			return null;
		}

		// -----------------------------------------------------------------
		// Other utilities
		// -----------------------------------------------------------------

		/**
		 * Convert to string.
		 */
		public String toString() {
			return "[" + key + " " + value + "]";
		}

		/**
		 * Debug print subtree.
		 */
		String debugPrint(String prefix) {
			return prefix + toString();
		}

		/**
		 * Validate tree's structure.
		 */
		void validate(Key minKey, Key maxKey) {
			if (minKey != null && key.compareTo(minKey) < 0) { // key must be >= minKey
				System.err.println("Validation error: Key is too small");
			}
			if (maxKey != null && key.compareTo(maxKey) >= 0) { // key must be < maxKey
				System.err.println("Validation error: Key is too large");
			}
		}

	}

	// -----------------------------------------------------------------
	// Private data
	// -----------------------------------------------------------------

	Node root; // root of tree
	int nEntries; // number of entries in dictionary

	// -----------------------------------------------------------------
	// Public methods
	// -----------------------------------------------------------------

	public AAXTree() { // constructor
		root = null;
		nEntries = 0;
	}

	/**
	 * Number of entries in the dictionary.
	 *
	 * @return Number of entries in the dictionary
	 */
	public int size() {
		return nEntries;
	}

	/**
	 * Find value of a given key.
	 *
	 * @param x The key to find
	 * @return The associated value if found
	 */
	public Value find(Key k) {
		if (root == null) {
			return null;
		} else {
			return root.find(k);
		}
	}

	/**
	 * Insert a key-value pair. Throws an exception if duplicate key found.
	 *
	 * @param x The key to insert
	 * @param v The associated value
	 * @throws Exception If the key is already in the tree
	 */
	public void insert(Key x, Value v) throws Exception {
		if (DEBUG) System.out.println("...Inserting " + x);
		if (root == null) {
			root = new ExternalNode(x, v);
		} else {
			root = root.insert(x, v);
		}
		
		nEntries += 1; // one more entry
		
		if (DEBUG) System.out.println("...Entire tree after insert(" + x + "): " + System.lineSeparator() + root.debugPrint("..."));
		if (VALIDATE) {
			validate(); // validate structure
		}
	}

	/**
	 * Delete a key.
	 *
	 * @param x The key to delete
	 * @throws Exception If the key is not in the tree
	 */
	public void delete(Key x) throws Exception {
		if (DEBUG) System.out.println("...Deleting " + x);
		if (root == null) {
			throw new Exception("Deletion of nonexistent key");
		} else {
			root = root.delete(x);
		}
		
		nEntries -= 1; // one fewer entry
		
		if (DEBUG) {
			if (root == null) {
				System.out.println("...Entire tree after delete(" + x + "): Empty tree");
			} else {
				System.out.println("...Entire tree after delete(" + x + "): " + System.lineSeparator() + root.debugPrint("..."));
			}
		}
		if (VALIDATE) {
			validate(); // validate structure
		}
	}

	/**
	 * Clear the tree, removing all entries.
	 *
	 */
	public void clear() {
		root = null;
		nEntries = 0;
		
		if (DEBUG) {
			if (root == null) {
				System.out.println("...Entire tree after clear: Empty tree");
			} 
			if (VALIDATE) {
				validate(); // validate structure
			}
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

	// -----------------------------------------------------------------
	// Enhanced operations
	// -----------------------------------------------------------------

	/**
	 * Get value associated with minimum key.
	 *
	 * @return A reference to the associated value, or null if tree empty
	 */
	public Value getMin() {
		if (root == null)
			return null;
		else
			return root.getMin();
	}

	/**
	 * Get value associated with maximum key.
	 *
	 * @return A reference to the associated value, or null if tree empty
	 */
	public Value getMax() {
		if (root == null)
			return null;
		else
			return root.getMax();
	}

	/**
	 * Find next value with key lesser or equal.
	 *
	 * @param x The key being sought
	 * @return A reference to the associated value
	 */
	public Value findSmaller(Key x) {
		if (root == null)
			return null;
		else
			return root.findSmaller(x);
	}

	/**
	 * Find next value of strictly greater key.
	 *
	 * @param x The key being sought
	 * @return A reference to the associated value
	 */
	public Value findLarger(Key x) {
		if (root == null)
			return null;
		else
			return root.findLarger(x);
	}

	/**
	 * Delete the entry with the smallest key.
	 *
	 * @return A reference to the associated value
	 */
	public Value removeMin() {
		if (root == null)
			return null;
		else {
			Value result = getMin();
			root = root.removeMin();
			nEntries -= 1; // one fewer entry
			validate(); // validate structure
			return result;
		}
	}

	/**
	 * Delete the entry with the largest key.
	 *
	 * @return A reference to the associated value
	 */
	public Value removeMax() {
		if (root == null)
			return null;
		else {
			Value result = getMax();
			root = root.removeMax();
			nEntries -= 1; // one fewer entry
			validate(); // validate structure
			return result;
		}
	}

	// -----------------------------------------------------------------
	// Debugging
	// -----------------------------------------------------------------

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

	/**
	 * Validate the tree's structure. This checks that the tree has
	 * the proper number of entries, keys are in proper order, and the
	 * AA-tree conditions are satisfied.
	 */
	void validate() {
		if (root == null) {
			if (nEntries != 0) System.err.println("Validation error: Incorrect number of entries (should be zero)");
		} else {
			root.validate(null, null);  // validate the structure
			ArrayList<String> list = root.getPreorderList(); 
			if (list.size() != 2 * nEntries - 1) { // number of nodes should be 2 * n - 1
				System.err.println("Validation error: Incorrect number of entries");
			}
		}
	}

}
