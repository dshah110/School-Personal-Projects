package model;

/**
 * This class represents the logic of a game where a board is updated on each
 * step of the game animation. The board can also be updated by selecting a
 * board cell.
 * 
 * @author Dept of Computer Science, UMCP
 */

public abstract class Game {
	protected BoardCell[][] board;
	private int maxRows;
	private int maxCols;

	/**
	 * Defines a board with BoardCell.EMPTY cells.
	 * 
	 * @param maxRows
	 * @param maxCols
	 */
	public Game(int maxRows, int maxCols) {
		this.maxRows = maxRows;
		this.maxCols = maxCols;
		board = new BoardCell[maxRows][maxCols];	//creates an empty board
		for(int row = 0; row < board.length; row ++) {
			for(int col = 0; col < board[row].length; col ++) {
				board[row][col] = BoardCell.EMPTY;
			}
		}
	}

	public int getMaxRows() { //returns rows
		return this.maxRows;
	}

	public int getMaxCols() { //returns columns
		return this.maxCols;
	}

	public void setBoardCell(int rowIndex, int colIndex, BoardCell boardCell) {
		board[rowIndex][colIndex] = boardCell; //sets a cell to boardCell
	}

	public BoardCell getBoardCell(int rowIndex, int colIndex) {
		return board[rowIndex][colIndex]; //returns board cell at specified index
	}

	/**
	 * Initializes row with the specified color.
	 * @param rowIndex
	 * @param cell
	 */
	public void setRowWithColor(int rowIndex, BoardCell cell) { //sets row with a color
		for(int row = 0; row<board.length; row ++) {
			for(int col = 0; col<board[rowIndex].length; col++) {
				board[rowIndex][col] = cell;
			}
		}
	}
	
	/**
	 * Initializes column with the specified color.
	 * @param colIndex
	 * @param cell
	 */
	public void setColWithColor(int colIndex, BoardCell cell) { //sets col  with a color
		for(int row = 0; row < board.length; row ++) {
			board[row][colIndex] = cell;
		}
	}
	
	/**
	 * Initializes the board with the specified color.
	 * @param cell
	 */
	public void setBoardWithColor(BoardCell cell) { //sets board with a color
		for(int row = 0; row < board.length; row ++) {
			for(int col = 0; col < board[row].length; col ++) {
				board[row][col] = cell;
			}
		}
	}	
	
	public abstract boolean isGameOver();

	public abstract int getScore();

	/**
	 * Advances the animation one step.
	 */
	public abstract void nextAnimationStep();

	/**
	 * Adjust the board state according to the current board state and the
	 * selected cell.
	 * 
	 * @param rowIndex
	 * @param colIndex
	 */
	public abstract void processCell(int rowIndex, int colIndex);
}