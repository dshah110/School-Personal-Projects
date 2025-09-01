package cmsc420_s21;

// YOU SHOULD NOT MODIFY THIS FILE

/**
 * Airport object. An airport is represented by its:
 *   - 3-letter IATA airport code (e.g., "LAX")
 *   - name (e.g., "Los Angeles International")
 *   - city (e.g., "Los Angeles")
 *   - country (e.g., "USA")
 *   - coordinates (x,y), given as longitude (-180...180) and latitude (-90...+90)
 */
public class Airport implements LabeledPoint2D, Comparable<Airport> {

	// Private data (The x,y coordinates are provided by the Point2D class)

	private final String code; // IATA code
	private final String name; // name
	private final String city; // city
	private final String country; // country
	private final Point2D location; // coordinates

	/**
	 * Basic constructor.
	 * 
	 * @param code    IATA code (String)
	 * @param name    airport's name (String)
	 * @param city    airport's city (String)
	 * @param country airport's country (String)
	 * @param x       x-coordinate [longitude] (double)
	 * @param y       y-coordinate [latitude] (double)
	 */
	public Airport(String code, String name, String city, String country, double x, double y) {
		this.code = code;
		this.name = name;
		this.city = city;
		this.country = country;
		this.location = new Point2D(x, y);
	}

	/**
	 * Minimal constructor. Constructs from just coordinates. This is used to refer
	 * to an arbitrary location in space.
	 * 
	 * @param x x-coordinate [longitude] (double)
	 * @param y y-coordinate [latitude] (double)
	 */
	public Airport(double x, double y) {
		this.code = "???";
		this.name = "No-Such-Aiport";
		this.city = "No-Such-City";
		this.country = "No-Such-Country";
		this.location = new Point2D(x, y);
	}

	// Standard functions - Getters and toString
	public String getCode() {
		return code;
	}

	public String getName() {
		return name;
	}

	public String getCity() {
		return city;
	}

	public String getCountry() {
		return country;
	}

	public String toString() {
		return code + ": " + location;
	}

	// Required by NamedPoint2D interface

	public String getLabel() {
		return code;
	}

	public double getX() {
		return location.getX();
	}

	public double getY() {
		return location.getY();
	}

	public double get(int i) {
		return location.get(i);
	}

	public Point2D getPoint2D() {
		return location;
	}
	
	/**
	 * Comparison function uses code for sorting.
	 * 
	 * @param ap Other airport
	 * @return Result of comparing codes
	 */
	public int compareTo(Airport ap) {
		return code.compareTo(ap.code); 
	}

	/**
	 * Generate string according to desired format.
	 * 
	 * @param format Formatting code
	 */
	public String getString(String format) {
		if (format.compareTo("default") == 0 || format.compareTo("") == 0) {
			return toString();
		} else if (format.compareTo("code-only") == 0) {
			return code;
		} else if (format.compareTo("full") == 0) {
			return code + ", " + name + ", " + city + ", " + country + ", (" + location.getX() + "," + location.getY()
					+ ")";
		} else if (format.compareTo("attributes") == 0) {
			return name + ", " + city + ", " + country + ", (" + location.getX() + "," + location.getY() + ")";
		} else
			return "Invalid format!";
	}

}