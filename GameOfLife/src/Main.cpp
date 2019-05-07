#include <SFML/Graphics.hpp>

constexpr int WIDTH = 1280;
constexpr int HEIGHT = 720;
constexpr int CELL_SIZE = 16;
constexpr int CELL_ROW_NUM = WIDTH / CELL_SIZE;
constexpr int CELL_COL_NUM = HEIGHT / CELL_SIZE;

struct Cell
{
	sf::Vector2i index;
	bool isAlive;

	Cell(sf::Vector2i index) : index(index), isAlive(false) {}
};

std::vector<Cell> createGrid();
void spawnAliveCells(std::vector<Cell>& cells);
void handleEvents(sf::RenderWindow& window, bool& isRunning);
void addOrRemoveCellWithMouse(std::vector<Cell>& cells, sf::RenderWindow& window);
void backUpCurrentState(std::vector<Cell>& cells, std::vector<bool>& currentState);
void generateNextState(std::vector<Cell>& cells, std::vector<bool>& currentState);
int getLivingNeighbours(int i, int j, std::vector<bool>& currentState);
void drawAliveCells(std::vector<Cell>& cells, sf::RenderWindow& window);

int main()
{
	srand((unsigned int)time(NULL));

	// Create window
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Game of Life", sf::Style::Titlebar | sf::Style::Close);
	window.setKeyRepeatEnabled(false);
	window.setFramerateLimit(30);

	// Create grid of cells
	std::vector<Cell> cells = createGrid();

	// Grid for checking if a cells lives for another generation
	std::vector<bool> currentState(CELL_ROW_NUM * CELL_COL_NUM);

	// Spawn alive cells at the beginning of simulation
	spawnAliveCells(cells);

	bool isRunning = false;

	while (window.isOpen())
	{
		handleEvents(window, isRunning);
		addOrRemoveCellWithMouse(cells, window);

		if (isRunning)
		{
			backUpCurrentState(cells, currentState);
			generateNextState(cells, currentState);
		}

		window.clear(sf::Color::Black);
		drawAliveCells(cells, window);
		window.display();
	}

	return 0;
}

std::vector<Cell> createGrid()
{
	std::vector<Cell> cells;
	cells.reserve(CELL_ROW_NUM * CELL_COL_NUM);
	for (int i = 0; i < CELL_ROW_NUM; i++)
	{
		for (int j = 0; j < CELL_COL_NUM; j++)
			cells.emplace_back(Cell(sf::Vector2i(i, j)));
	}
	return cells;
}

void spawnAliveCells(std::vector<Cell>& cells)
{
	int aliveCellsNum = 8000 / CELL_SIZE;

	for (int i = 0; i < aliveCellsNum; i++)
	{
		int randomX = rand() % CELL_ROW_NUM;
		int randomY = rand() % CELL_COL_NUM;
		cells[randomX * CELL_COL_NUM + randomY].isAlive = true;
	}
}

void handleEvents(sf::RenderWindow& window, bool& isRunning)
{
	sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			window.close();

		if (event.type == sf::Event::KeyPressed)
		{
			// Play/Pause Simulation
			if (event.key.code == sf::Keyboard::Space)
				isRunning = !isRunning;
		}
	}
}

void addOrRemoveCellWithMouse(std::vector<Cell>& cells, sf::RenderWindow& window)
{
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Right))
	{
		int x = sf::Mouse::getPosition(window).x / CELL_SIZE;
		int y = sf::Mouse::getPosition(window).y / CELL_SIZE;

		// Only toggle cell if mouse is inside window
		if (x >= 0 && y >= 0 && x < CELL_ROW_NUM && y < CELL_COL_NUM)
		{
			// Make cells alive
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				cells[x * CELL_COL_NUM + y].isAlive = true;

			// Make cells dead
			if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
				cells[x * CELL_COL_NUM + y].isAlive = false;
		}
	}
}

void backUpCurrentState(std::vector<Cell>& cells, std::vector<bool>& currentState)
{
	for (int i = 0; i < CELL_ROW_NUM; i++)
	{
		for (int j = 0; j < CELL_COL_NUM; j++)
			currentState[i * CELL_COL_NUM + j] = cells[i * CELL_COL_NUM + j].isAlive;
	}
}

void generateNextState(std::vector<Cell>& cells, std::vector<bool>& currentState)
{
	for (int i = 0; i < CELL_ROW_NUM; i++)
	{
		for (int j = 0; j < CELL_COL_NUM; j++)
		{
			int livingNeighbours = getLivingNeighbours(i, j, currentState);
			int index = i * CELL_COL_NUM + j;

			// Conditions for a cell to live
			bool aliveCellWIth2or3Neighbours = currentState[index] && (livingNeighbours == 2 || livingNeighbours == 3);
			bool deadCellWith3Neighbours = !currentState[index] && livingNeighbours == 3;
			cells[index].isAlive = aliveCellWIth2or3Neighbours || deadCellWith3Neighbours;
		}
	}
}

int getLivingNeighbours(int i, int j, std::vector<bool>& currentState)
{
	int livingNeighbours = 0;

	// Ignore border conditions
	if (i != 0 && i != CELL_ROW_NUM - 1 && j != 0 && j != CELL_COL_NUM - 1)
	{
		// Count alive neighbors from a 3x3 grid surrounding the cell being checked
		for (int k = i - 1; k <= i + 1; k++)
		{
			for (int l = j - 1; l <= j + 1; l++)
			{
				// Don't count the cell being checked as neighbour
				if (k == i && j == l)
					continue;

				// Increase counter if a living neighbour is found
				if (currentState[k * CELL_COL_NUM + l])
					livingNeighbours++;

				// Having 4 or more neighbours is irrelevant
				if (livingNeighbours == 4)
					return livingNeighbours;
			}
		}
	}

	return livingNeighbours;
}

void drawAliveCells(std::vector<Cell>& cells, sf::RenderWindow & window)
{
	for (int i = 0; i < CELL_ROW_NUM; i++)
	{
		for (int j = 0; j < CELL_COL_NUM; j++)
		{
			int index = i * CELL_COL_NUM + j;
			if (cells[index].isAlive)
			{
				sf::CircleShape cell(CELL_SIZE / 2);
				cell.setPosition((sf::Vector2f)(cells[index].index * CELL_SIZE));
				cell.setOutlineColor(sf::Color::Red);
				cell.setOutlineThickness(1.0f);

				window.draw(cell);
			}
		}
	}
}