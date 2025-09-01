package cmsc420_s21; // Don't delete this or your file won't pass the autograder

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.TreeMap;

/**
 * Tour (skeleton)
 *
 * MODIFY THE FOLLOWING CLASS.
 *
 * You are free to make whatever changes you like or to create additional
 * classes and files.
 */

public class Tour<LPoint extends LabeledPoint2D> {
//	private AAXTree<String, Integer> locator; // locator structure
	private TreeMap<String, Integer> locator;
	private ArrayList<LPoint> tour; // the tour
	private WKDTree<LPoint> spatialIndex;
	
	public Tour() {
		locator = new TreeMap<String, Integer>();
		//locator = new AAXTree<String, Integer>();
		tour = new ArrayList<LPoint>(); //need to change this to AAXTree;
		spatialIndex = new WKDTree<LPoint>();
	}
	
	public void append(LPoint pt) throws Exception { 
		for(LPoint p : tour) {
			if(p.getLabel().equals(pt.getLabel())) {
				throw new Exception("Duplicate label");
			}
			if(p.getX() == pt.getX() && p.getY() == pt.getY()) {
				throw new Exception("Duplicate coordinates");
			}
		}
		tour.add(pt);
		locator.put(pt.getLabel(), tour.indexOf(pt));
		//locator.put(pt.getLabel(), tour.indexOf(pt));
		/**ADD THE POINT TO THE SPATIAL INDEX (WKDTREE METHOD)**/
	}
	
	public ArrayList<LPoint> list() { 
		return tour;
	}
	
	public void clear() { 
		tour.clear();
		locator.clear();
		/**CLEAR SPATIAL INDEX THING**/
	}
	
	public double cost() { 
		double cost = 0.0;
		for(int i = 0; i < tour.size()/* == n*/; i++) {
			int nextPoint = (i+1)%tour.size();
			cost += tour.get(i).getPoint2D().distanceSq(tour.get(nextPoint).getPoint2D());
		}
		return cost;
	}
	
	public void reverseHelper(String label1, String label2) throws Exception {
		List<LPoint> sublst;
		int index1 = locator.get(label1); int index2 = locator.get(label2);
		if(index1 > index2) {
			sublst = tour.subList(index2 + 1, index1 + 1);
		}
		else {
			sublst = tour.subList(index1 + 1, index2 + 1);
		}
		Collections.reverse(sublst);
		for(int i = 0; i < tour.size(); i++) {
			locator.put(tour.get(i).getLabel(), i);
		}
	}
	
	
	public void reverse(String label1, String label2) throws Exception { 
		if(!locator.containsKey(label1) || !locator.containsKey(label2) ) {
			throw new Exception("Label not found");
		}
		else if(label1.equals(label2)) {
			throw new Exception("Duplicate label");
		}
		else {
			reverseHelper(label1, label2);
		/*	int index1 = locator.find(label1);
			int index2 = locator.find(label2);
			List<LPoint> sublst;
			if(index1 < index2) {
				sublst = tour.subList(index1 + 1, index2 + 1);
			}
			else {
				sublst = tour.subList(index2 + 1, index1 );
			}
			Collections.reverse(sublst);
			for(LPoint p : sublst) {
				if(index1 < index2) {
					locator.replace(p.getLabel(), sublst.indexOf(p) + index1);
				}
				else {
					locator.replace(p.getLabel(), sublst.indexOf(p) + index2);
				}
			} */
		} 
	} // Reverse subtour
	/*
	public void reverse(String label1, String label2) throws Exception { 
		if(locator.find(label1) == null || locator.find(label2) == null) {
			throw new Exception("Label not found");
		}
		else if(label1.equals(label2)) {
			throw new Exception("Duplicate label");
		}
		else {
			int index1 = locator.find(label1);
			int index2 = locator.find(label2);
			if(index1 > index2) {
				int temp = index2;
				index2 = index1;
				index1 = temp;
			}

			List<LPoint> sublst;
			sublst = tour.subList(index1 + 1, index2 + 1); //WE'RE ADDING A +1 TO INDEX1 B/C WE'RE REVERSING THE LIST FROM I+1 TO J
			//AND WE DO INDEX2+1 BC SUBLIST IS NOT AN INCLUSIVE OPERATION
			Collections.reverse(sublst);
			for(LPoint p : sublst) {
				locator.replace(p.getLabel(), sublst.indexOf(p) + index1); 	//IT MAY BE THE CASE THAT WE ADD ONE HERE
				//THAT IS SUBLST.INDEXOF(P) + INDEX1 + 1
			}
		}
	}*/
	
	
	
	public boolean twoOpt(String label1, String label2) throws Exception { 
		if(locator.get(label1) == null || locator.get(label2) == null) {
			throw new Exception("Label not found");
		}
		else if(label1.equals(label2)) {
			throw new Exception("Duplicate label");
		}
		else {
			int index1 = locator.get(label1);
			int index2 = locator.get(label2);
			
			int index1np = (index1 + 1)%tour.size();
			int index2np = (index2 + 1)%tour.size();
			double part1 = tour.get(index1).getPoint2D().distanceSq(tour.get(index2).getPoint2D()) + 
						   tour.get(index1np).getPoint2D().distanceSq(tour.get(index2np).getPoint2D());
			double part2 = tour.get(index1).getPoint2D().distanceSq(tour.get(index1np).getPoint2D()) + 
					   	   tour.get(index2).getPoint2D().distanceSq(tour.get(index2np).getPoint2D());
			double delta = part1 - part2;
			if(this.cost() + delta < this.cost()) {
				reverse(label1, label2);
				return true;
			}
			else {
				return false;
			}
		}
	} 
	
	public LPoint twoOptNN(String label) throws Exception { 
		/**WE NEED FIXEDDNNRADIUS FOR THIS. WORK ON WKDTREE FIRST AND THEN COME BACK TO THIS**/
		if(locator.get(label) == null) {
			throw new Exception("Label not found");
		}
		else {
			int index = locator.get(label);
			LPoint nn = spatialIndex.fixedRadNN(tour.get(index).getPoint2D(), tour.get(index).getPoint2D().distanceSq(tour.get(index + 1).getPoint2D()));
			if(nn == null) {
				return null;
			}
			else {
				boolean opt2 = twoOpt(label, nn.getLabel());
				return opt2 == true ? nn : null;
			}
		}
	}
	
	public int allTwoOpt() { 
		int count = 0;
		for(int i = 0; i < tour.size(); i++) {
			for(int j = i + 1; j < tour.size(); j++) {
				try {
					if(twoOpt(tour.get(i).getLabel(),tour.get(j).getLabel()) == true) {
						count++;
					}
				} catch (Exception e) {}
				
			}
		}
		return count;
	}
	
	/**CHECK AND MAKE SURE MY REPLACE FUNCTION WORKS THE WAY IT SHOULD.**/
	
	
}
