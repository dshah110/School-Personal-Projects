package cmsc420_s21; // Don't delete this or your file won't pass the autograder

import java.util.ArrayList;

public class AAXTree<Key extends Comparable<Key>, Value> {
	Node root;
	int size;

	private abstract class Node { // generic node (purely abstract)
		abstract Value find(Key k);
		abstract Node insert(Key k, Value v) throws Exception;
		abstract Node getLeft();
		abstract Node getRight();
		abstract int getLevel();
		abstract void setLeft(Node v);
		abstract void setRight(Node v);
		abstract void setLevel(int l);
		abstract Node skew();
		abstract Node split();
		abstract void updateHeight();
		abstract Node fixAfterDelete();
		public abstract String toString();
		abstract Value getMin();
		abstract Value getMax();
		abstract Value findSmaller(Key x, Node child);
		abstract Value findLarger(Key x, Node child);
		abstract Node delete(Key x) throws Exception;
		abstract Key getMinKey();
		abstract Key getMaxKey();
	}

	private class InternalNode extends Node { // internal node
		Key key;
		Node left, right;
		int level;

		public InternalNode(Key k, Node l, Node r, int lvl) {
			key = k;
			left = l;
			right = r;
			level = lvl;
		}
		
		Value find(Key x) {
			int compVal = x.compareTo(key);
			if(compVal < 0) {
				return left.find(x);
			}
			else {
				return right.find(x);
			}
		}

		Node insert(Key k, Value v) throws Exception {
			int compVal = k.compareTo(key);
			if(compVal < 0) {
				left = left.insert(k, v);
			}
			else {
				right = right.insert(k, v);
			}
			return skew().split();
		}
	
		Node getLeft() {
			// TODO Auto-generahojn;lkted method stub
			return left;
		}

		Node getRight() {
			// TODO Auto-generated method stub
			return right;
		}
	
		int getLevel() {
			// TODO Auto-generated method stub
			return level;
		}

		void setLeft(Node v) {
			left = v;
			
		}

		void setRight(Node v) {
			// TODO Auto-generated method stub
			right = v;
		}

		void setLevel(int l) {
			// TODO Auto-generated method stub
			level = l;
		}

		Node skew() {
			if(this.left.getLevel() == this.getLevel()) {
				Node q = this.getLeft();
				this.setLeft(q.getRight());
				q.setRight(this);
				return q;
			}
			else { 
				return this;
			}
		}

		Node split() {
			if(this.getRight().getRight().getLevel() == this.getLevel()) {
				Node q = this.getRight();
				this.setRight(q.getLeft());
				q.setLeft(this);
				q.setLevel(q.getLevel() + 1);
				return q;
			}
			else {
				return this;
			}
		}

		void updateHeight() {
			int lvl = 1 + Math.min(this.getLeft().getLevel(), this.getRight().getLevel());
			if(this.getLevel() > lvl) {
				this.setLevel(lvl);
			}
		}
	
		public String toString() {
			return "(" + key + ") " + level;
		}

		Node delete(Key x) throws Exception {
			int compVal = x.compareTo(this.key);
			Node result;
			if(compVal < 0) {
				result = left.delete(x);
				if(result == null) {
					Node p = this.getRight();
					return p;
				}
				else {
				//	this.setLeft(result);
					this.setLeft(result);
					return this.fixAfterDelete();
					//RETURN FIXAFTERDELETE
				}  
			}
			else {

				result = right.delete(x);
				if(result == null) {
					Node p = this.getLeft();
					return p;
				}
				else {
					this.setRight(result);
					return this.fixAfterDelete();
				}  
			}	
		} 


		Node fixAfterDelete() {
			Node p = this;
			p.updateHeight();
			p = p.skew(); 
			p.setRight(p.getRight().skew()); p.getRight().setRight(p.getRight().getRight().skew());
			p = p.split(); p.setRight(p.getRight().split());
			return p;
			
		}


		Value getMin() {
			return this.getLeft().getMin();
		}

		Value getMax() {
			return this.getRight().getMax();
		}

		Value findSmaller(Key x, Node child) {
			Value result;
			Node llchild = child;
			int compVal = x.compareTo(key);
			if(compVal > 0) {
				if(this instanceof AAXTree.InternalNode) {
					llchild = this;
				}
				result = right.findSmaller(x, llchild);
			}
			else {
				result = left.findSmaller(x, llchild);
			}
			if(result != null) {
				return result;
			}
			else {
				return llchild.getLeft().getMax();
			}
		}


		Value findLarger(Key x, Node child) {
			Value result;
			Node llchild = child;
			int compVal = x.compareTo(key);
			if(compVal < 0) {
				if(this instanceof AAXTree.InternalNode) {
					llchild = this;
				}
				result = left.findLarger(x, llchild);
			}
			else {
				result = right.findLarger(x, llchild);
			}
			if(result != null) {
				return result;
			}
			else {
				return llchild.getRight().getMin();
			}
		}

		
		Key getMinKey() {
			return this.getLeft().getMinKey();
		}

		Key getMaxKey() {
			return this.getRight().getMaxKey();
		}

	}

	
	
	
	

	
	private class ExternalNode extends Node { // external node
		Key key;
		Value value;
		
		public ExternalNode(Key k, Value v) {
			key = k;
			value = v;
		}

		Value find(Key x) {
			if(x.compareTo(key) == 0) {
				return value;
			}
			else {
				return null;
			}
		}

		Node insert(Key k, Value v) throws Exception  {
			int compVal = k.compareTo(key);
			if(compVal == 0) {
				throw new Exception("Insertion of duplicate key");
			}
			else {
				ExternalNode node = new ExternalNode(k, v);
				InternalNode p;
				if(compVal < 0) {
					p = new InternalNode(key, node, this, 1);
				}
				else {
					p = new InternalNode(k, this, node, 1);
				}
				size++;
				return p;
			}			
		}

		Node getLeft() {
			// TODO Auto-generated method stub
			return this;
		}

		Node getRight() {
			// TODO Auto-generated method stub
			return this;
		}

		int getLevel() {
			// TODO Auto-generated method stub
			return 0;
		}

		void setLeft(Node v) {}
		
		void setRight(Node v) {}

		void setLevel(int l) {}

		Node skew() {
			return this;
		}

		Node split() {
			return this;
		}

		void updateHeight() {}

		public String toString() {
			return "[" + key + " " + value + "]";
		}
	
		Node delete(Key x) throws Exception {
			if(x.compareTo(this.key) != 0) {
				throw new Exception("Deletion of nonexistent key");
			}
			else {
				size--;
				return null;
			}
		}
		
		Node fixAfterDelete() {return this;}

		Value getMin() {
			return value;
		}

		Value getMax() {
			return value;
		}

		Value findSmaller(Key x, Node child) {
			int compVal = x.compareTo(key);
			if(compVal < 0) {
				return child.getLeft().getMax();
			}
			else if(compVal > 0) {
				return this.value;
			}
			else {
				return null;
			}
		}


		Value findLarger(Key x, Node child) {
			int compVal = x.compareTo(key);
			if(compVal < 0) {
				return this.value;
			}
			else if(compVal > 0) {
				return child.getRight().getMin();
			}
			else {
				return null;
			}
		}

		
		Key getMinKey() {
			// TODO Auto-generated method stub
			return key;
		}

		
		Key getMaxKey() {
			// TODO Auto-generated method stub
			return key;
		}


	}

	//------------------------------------------------------------------
	// Needed for Part a
	//------------------------------------------------------------------
	public AAXTree() {
		root = null;
		size = 0;
	}
	
	public Value find(Key k) {  /* ... */ 
		if(root == null) {
			return null;
		}
		else {
			return root.find(k);
		}
	}
	
	public void insert(Key k, Value v) throws Exception { 
		if(root == null) {
			ExternalNode node = new ExternalNode(k, v);
			root = node;
			size++;
		}
		else {
			root = root.insert(k, v);
		}
	}
	
	public void clear() { 
		root = null;
		size = 0;
	}
	
	public ArrayList<String> getPreorderList() { 
		ArrayList<String> list = new ArrayList<String>();
		preOrder(root, list);
		return list;
	}
	
	private void preOrder(Node node, ArrayList<String> list) {
		if(node == null) {
			return;
		}
		if(node.getLeft() == node && node.getRight() == node) {
			list.add(node.toString());
			return;
		}
		list.add(node.toString());
		preOrder(node.getLeft(), list);
		preOrder(node.getRight(), list);
	}

	//------------------------------------------------------------------
	// Needed for Part b
	//------------------------------------------------------------------
	public void delete(Key x) throws Exception { 
		if(root == null) {
			throw new Exception("Deletion of nonexistent key");
		}
		else {
			root = root.delete(x);
		}
	}
	
	public int size() { return size; }
	
	public Value getMin() { return (root == null) ? null : root.getMin(); }
	
	public Value getMax() { return (root == null) ? null : root.getMax(); }
	
	public Value findSmaller(Key x) { 
		if(root == null || x.compareTo(root.getMinKey()) <= 0) {
			return null;
		}
		else {
			return root.findSmaller(x, root);
		}
	}
	
	public Value findLarger(Key x) { 
		if(root == null || x.compareTo(root.getMaxKey()) >= 0) {
			return null;
		}
		else {
			return root.findLarger(x, root);
		}
	}
	

	public Value removeMin() { 
		if(root == null) {
			return null;
		}
		
		Value result = root.getMin();
		try {
			root = root.delete(root.getMinKey());
		}
		catch(Exception e) {}
		return result;
	}
	
	public Value removeMax() {
		if(root == null) {
			return null;
		}
		Value result = root.getMax();
		try {
			root = root.delete(root.getMaxKey());
		}
		catch(Exception e) {}
		return result;
	}

	
	
/*	public static void main(String[] args) throws Exception {
		AAXTree<Integer, Character> tree = new AAXTree<Integer, Character>();
		tree.delete(5);
		System.out.println(tree.removeMax());
		tree.delete(6);
		tree.insert(72, 'z');
		tree.insert(5, 'e');
		tree.insert(6, 'f');
		tree.insert(4, 'h');
		tree.insert(7, 'v');
		tree.insert(10, 'c');
		tree.insert(2, 'q');
		tree.delete(7); 
		System.out.println(tree.getPreorderList());
		System.out.println(tree.removeMin());
		System.out.println(tree.removeMax());
		tree.delete(4);
		System.out.println(tree.getPreorderList());
		tree.delete(72);
		tree.delete(-1);
		tree.removeMin();

		System.out.println(tree.size());
		System.out.println(tree.findLarger(71));

		
		System.out.println(tree.removeMin());
		System.out.println(tree.find(2));
		System.out.println(tree.getMaxKey());
		System.out.println(tree.findSmaller(7)); //should be q
		System.out.println(tree.findLarger(3)); // should be h
	}  
*/
} 
