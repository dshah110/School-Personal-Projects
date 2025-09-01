package cmsc420_s21;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map.Entry;
import java.util.TreeMap;

// -------------------------------------------------------------------------
// Add your code here. You may import whatever (standard) packages you like.
// You may also create additional files, if you like, but be sure to upload
// them as well for grading.
// -------------------------------------------------------------------------

public class Tour<Point extends LabeledPoint2D> {

	// -------------------------------------------------------------------------
	// Here is the public interface. You should not change this, but you
	// can add any additional methods and/or data members that you like.
	// -------------------------------------------------------------------------
	private ArrayList<Point> tour;
	private TreeMap<String, Integer> tourmap;

	public Tour() { 
		tour = new ArrayList<Point>();
		tourmap = new TreeMap<String, Integer>();
	} // Constructor

	public String append(Point pt) {
		String label = pt.getLabel();
		StringBuffer result = new StringBuffer();
		if(!tourmap.containsKey(label)) {
			tour.add(pt);
			tourmap.put(label, tour.indexOf(pt)); ///MAYBE YOU CAN ADD THE POINT INSTEAD OF THE INDEX OF THE POINT 
			result.append("append(" + label + "): Added to tour at index " + tour.indexOf(pt));
			return result.toString();
		}
		else {
			return "append(" + label + "): " +"Error - Label exists (operation ignored)";
		}
				
	} // Append point to end of tour

	public String listTour() { 
		StringBuffer result = new StringBuffer("list-tour:");
		String tourString = "";
		for(Point pt : tour) {
			tourString += " " + tour.indexOf(pt) + ":" + pt.getLabel();
		}
		result.append(tourString);
		return result.toString(); 
	} // List the labels in tour order

	public String listLabels() { 
		StringBuffer result = new StringBuffer("list-labels:");
		StringBuffer labelString = new StringBuffer("");
		for(Entry<String, Integer> label: tourmap.entrySet()) {
			labelString.append(" ");
			labelString.append(label.getKey());
			labelString.append(":");
			labelString.append(label.getValue());
		}

		result.append(labelString);
		return result.toString(); 
	} // List the labels in alphabetic order
	


	public String indexOf(String label) { 
		StringBuffer result = new StringBuffer("index-of(");
		result.append(label);
		result.append("): ");
		if(tourmap.containsKey(label)) {
			result.append(tourmap.get(label));
		}
		else {
			result.append("Not-found");
		}
		return result.toString();
	} // Index of label in tour

	public String reverse(String label1, String label2) { 
		StringBuffer output = new StringBuffer("reverse(" + label1 + "," + label2 +"): ");
		String err = "";
		if(!tourmap.containsKey(label1) || !tourmap.containsKey(label2)) {
			if(!tourmap.containsKey(label1)) {
				err = label1;
			}
			else if(!tourmap.containsKey(label2)) {
				err = label2;
			}
			output.append("Error - Label " + err + " does not exist (operation ignored)");
		}
		else if(label1.equals(label2)) {
			output.append("Error - Labels are equal (operation ignored)");
		}
		else {
			int index1 = tourmap.get(label1);
			int index2 = tourmap.get(label2);
			int k;
			List<Point> sublst;
			if(index1 < index2) {
				sublst = tour.subList(index1, index2 + 1);
				k = index2 - index1 + 1;
			}
			else {
				sublst = tour.subList(index2, index1 + 1);
				k = index1 - index2 + 1;
			}
			Collections.reverse(sublst);
			for(Point p : sublst) {
				if(index1 < index2) {
					tourmap.replace(p.getLabel(), sublst.indexOf(p) + index1);
				}
				else {
					tourmap.replace(p.getLabel(), sublst.indexOf(p) + index2);
				}
			}
			output.append("Successfully reversed subtour of length ");
			output.append(k);
		}
		return output.toString();
	} // Reverse subtour
}
