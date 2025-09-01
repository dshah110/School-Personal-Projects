
package model;

import java.awt.Color;
import java.util.Random;

/* This class must extend Game */
public class ClearCellGame extends Game {
	private Random random;
	private int strategy;
	private int score=0;
	
	public ClearCellGame(int maxRows, int maxCols, Random random, int strategy) {
		super(maxRows, maxCols);	//initializes everything
		this.random = random;
		this.strategy = strategy;
	}

	@Override
	public boolean isGameOver() {
		return !isRowEmpty(board.length - 1); //if row is  empty, game is over
	}

	@Override
	public int getScore() {
		return score; //returns score
	}
	
	@Override
	public void nextAnimationStep() {
		if (isRowEmpty(board.length - 1)) { //if row is empty
			// swap
			for (int i=board.length-1; i > 0; i--) {
				swapBoardRows(i, (i-1));
			}
			// set new row
			for (int i=0; i < board[0].length; i++) {
				setBoardCell(0, i, BoardCell.getNonEmptyRandomBoardCell(this.random));
			}
		}
	} 
	
	@Override
	public void processCell(int rowIndex, int colIndex) {
		Color colorToClear = board[rowIndex][colIndex].getColor();
		if(board[rowIndex][colIndex] != BoardCell.EMPTY) { //if index is not empty,
			board[rowIndex][colIndex] = BoardCell.EMPTY;//sets it to empty
			score++;
		}
		//clears all directions
		processCellTopRight(rowIndex, colIndex, colorToClear);
		processCellTopLeft(rowIndex, colIndex, colorToClear);
		processCellTop(rowIndex, colIndex, colorToClear);
		processCellRight(rowIndex, colIndex, colorToClear);
		processCellBottomRight(rowIndex, colIndex, colorToClear);
		processCellBottom(rowIndex, colIndex, colorToClear);
		processCellBottomLeft(rowIndex, colIndex, colorToClear);
		processCellLeft(rowIndex, colIndex, colorToClear); 
		int i = 0;
		while(i<thereIsAnEmptyRow()) {	//collapses if there are numerous empty rows
			collapse();
			i++;
		}
	}
	
	private void processCellTopRight(int i, int j, Color c) {
		try {	//if the top right is empty and its the same color as the middle cell
			if (board[i+1][j+1] != BoardCell.EMPTY && board[i+1][j+1].getColor() == c) {
				board[i+1][j+1] = BoardCell.EMPTY;  //it sets it as empty
				processCellTopRight(i+1, j+1, c); //recursively calls this method
				score++; //increases score
			}
		} catch (ArrayIndexOutOfBoundsException e) { //catches the exception
			;
		}
	}
	private void processCellBottomRight(int i, int j, Color c) { //clears bottom right
		try {
			if (board[i-1][j+1] != BoardCell.EMPTY && board[i-1][j+1].getColor() == c) {
				board[i-1][j+1] = BoardCell.EMPTY;
				processCellBottomRight(i-1, j+1, c);
				score++;
			}
		} catch (ArrayIndexOutOfBoundsException e) {
			;
		}
	}
	private void processCellTopLeft(int i, int j, Color c) { //clears top left
		try {
			if (board[i+1][j-1] != BoardCell.EMPTY && board[i+1][j-1].getColor() == c) {
				board[i+1][j-1] = BoardCell.EMPTY;
				processCellTopLeft(i+1, j-1, c);
				score++;
			}
		} catch (ArrayIndexOutOfBoundsException e) {
			;
		}
	}
	private void processCellBottomLeft(int i, int j, Color c) { //clears bottom left
		try {
			if (board[i-1][j-1] != BoardCell.EMPTY && board[i-1][j-1].getColor() == c) {
				board[i-1][j-1] = BoardCell.EMPTY;
				processCellBottomLeft(i-1, j-1, c);
				score++;
			}
		} catch (ArrayIndexOutOfBoundsException e) {
			;
		}
	}
	private void processCellTop(int i, int j, Color c) { //clears up
		try {
			if (board[i+1][j] != BoardCell.EMPTY && board[i+1][j].getColor() == c) {
				board[i+1][j] = BoardCell.EMPTY;
				processCellTop(i+1, j, c);
				score++;
			}
		} catch (ArrayIndexOutOfBoundsException e) {
			;
		}
	}
	private void processCellBottom(int i, int j, Color c) { //clears down
		try {
			if (board[i-1][j] != BoardCell.EMPTY && board[i-1][j].getColor() == c) {
				board[i-1][j] = BoardCell.EMPTY;
				processCellBottom(i-1, j, c);
				score++;
			}
		} catch (ArrayIndexOutOfBoundsException e) {
			;
		}
	}
	private void processCellRight(int i, int j, Color c) {  //clears right
		try {
			if (board[i][j+1] != BoardCell.EMPTY && board[i][j+1].getColor() == c) {
				board[i][j+1] = BoardCell.EMPTY;
				processCellRight(i, j+1, c);
				score++;
			}
		} catch (ArrayIndexOutOfBoundsException e) {
			;
		}
	}
	private void processCellLeft(int i, int j, Color c) {  //clears  left
		try {
			if (board[i][j-1] != BoardCell.EMPTY && board[i][j-1].getColor() == c) {
				board[i][j-1] = BoardCell.EMPTY;
				processCellLeft(i, j-1, c);
				score++;
			}
		} catch (ArrayIndexOutOfBoundsException e) {
			;
		}
	}
	private void collapse() {
		for(int i=0; i < board.length-1; i++) {
			if (isRowEmpty(i)) {		//brings empty row down	
				swapBoardRows(i, i+1);
			}
		}
	}	
	private boolean isRowEmpty(int rowIndex) { //if row is empty, returns true
		for(BoardCell c: board[rowIndex]) {
			if (c != BoardCell.EMPTY) {
				return false;
			}
		}
		return true;
	}
	private void swapBoardRows(int i, int j) { //swaps rows
		BoardCell[] tempArray = board[i];
		board[i] = board[j];
		board[j] = tempArray;
	}
	private int thereIsAnEmptyRow() { //returns the number of empty rows 
		int  count = 0;
		for(int row = 0; row<board.length-1; row++) {
			if(isRowEmpty(row)) {
				count++;
			}
		}
		return count;
	}

}