package cmsc420_s21;

// YOU SHOULD NOT MODIFY THIS FILE

import java.util.ArrayList;
import java.util.Scanner;

/**
 * Command handler. This inputs a single command line, processes the command (by
 * invoking the appropriate method(s) on the tour.
 */

public class CommandHandler {

	private Tour<Airport> tour; // the tour
	private double priorCost; // previous cost value

	/**
	 * Initialize command handler
	 */
	public CommandHandler() {
		tour = new Tour<Airport>();
		priorCost = -1; // default value
	}

	/**
	 * Process a single command and return the string output. Each line begins with
	 * a command (e.g., find, insert, delete) followed by a list of arguments. The
	 * arguments are separated by colons (":").
	 * 
	 * @param inputLine The input line with the command and parameters.
	 * @return A short summary of the command's execution/result.
	 */

	public String processCommand(String inputLine) throws Exception {
		String output = new String(); // for storing summary output
		Scanner line = new Scanner(inputLine);
		try {
			line.useDelimiter(":"); // use ":" to separate arguments
			String cmd = (line.hasNext() ? line.next() : ""); // next command

			// -----------------------------------------------------
			// INSERT code name city country x y
			// -----------------------------------------------------
			if (cmd.compareTo("append") == 0) {
				String code = line.next(); // get parameters
				String name = line.next();
				String city = line.next();
				String country = line.next();
				double x = line.nextDouble();
				double y = line.nextDouble();
				Airport ap = new Airport(code, name, city, country, x, y); // create airport object
				output += "append(" + code + "): ";
				tour.append(ap); // append it
				output += "successful {" + ap.getString("attributes") + "}" + System.lineSeparator();
			}
			// -----------------------------------------------------
			// LIST - list the points of the tour
			// -----------------------------------------------------
			else if (cmd.compareTo("list-tour") == 0) {
				ArrayList<Airport> list = tour.list();
				if (list == null) throw new Exception("Error - list returned a null result");
				output += "list-tour:" + System.lineSeparator();
				for (int i = 0; i < list.size(); i++) { // output the list
					output += "  [" + i + "] " + list.get(i) + System.lineSeparator();
				}
				output += "tour-short-summary:";
				for (int i = 0; i < list.size(); i++) {
					output += " " + i + ":" + list.get(i).getLabel();
				}
				output += System.lineSeparator();
			}
			// -----------------------------------------------------
			// CLEAR
			// -----------------------------------------------------
			else if (cmd.compareTo("clear") == 0) {
				tour.clear(); // get the tour's cost
				priorCost = -1; // reset prior cost
				output += "clear: successful" + System.lineSeparator();
			}
			// -----------------------------------------------------
			// COST
			// -----------------------------------------------------
			else if (cmd.compareTo("cost") == 0) {
				double cost = tour.cost(); // get the tour's cost
				output += "cost: " + cost;
				if (priorCost >= 0) {
					output += " (change from prior cost = " + (cost - priorCost) + ")" + System.lineSeparator();
				} else {
					output += System.lineSeparator();
				}
				priorCost = cost; // save this cost
			}
			// -----------------------------------------------------
			// REVERSE code1 code2
			// -----------------------------------------------------
			else if (cmd.compareTo("reverse") == 0) {
				String code1 = line.next(); // get parameters
				String code2 = line.next();
				output += "reverse(" + code1 + "," + code2 + "): ";
				tour.reverse(code1, code2); // apply the operation
				output += "successful" + System.lineSeparator();
			}
			// -----------------------------------------------------
			// TWO-OPT code1 code2
			// -----------------------------------------------------
			else if (cmd.compareTo("two-opt") == 0) {
				String code1 = line.next(); // get parameters
				String code2 = line.next();
				output += "two-opt(" + code1 + "," + code2 + "): ";
				boolean effective = tour.twoOpt(code1, code2); // apply the operation
				output += (effective ? "effective" : "ineffective") + System.lineSeparator();
			}
			// -----------------------------------------------------
			// TWO-OPT-NN code
			// -----------------------------------------------------
			else if (cmd.compareTo("two-opt-nn") == 0) {
				String code = line.next(); // get parameters
				output += "two-opt-nn(" + code + "): ";
				Airport ap = tour.twoOptNN(code); // apply the operation
				if (ap != null) {
					output += "effective (with " + ap.getCode() + ")" + System.lineSeparator();
				} else {
					output += "ineffective" + System.lineSeparator();
				}
			}
			// -----------------------------------------------------
			// ALL-TWO-OPT-NN
			// -----------------------------------------------------
			else if (cmd.compareTo("all-two-opt") == 0) {
				output += "all-two-opt: ";
				int effectiveCount = tour.allTwoOpt(); // apply the operation
				output += "successful (" + effectiveCount + " effective)" + System.lineSeparator();
			}
			// 
			// -----------------------------------------------------
			// Invalid command or empty
			// -----------------------------------------------------
			else {
				if (cmd.compareTo("") == 0)
					System.err.println("Error: Empty command line (Ignored)");
				else
					System.err.println("Error: Invalid command - \"" + cmd + "\" (Ignored)");
			}
		} catch (Exception e) { // exception thrown?
			output += "failure due to exception: \"" + e.getMessage() + "\"" + System.lineSeparator();
		} catch (Error e) { // error occurred?
			System.err.print("Operation failed due to error: " + e.getMessage());
			e.printStackTrace(System.err);
		} finally { // always executed
			line.close(); // close the input scanner
		}
		return output; // return summary output
	}
}
