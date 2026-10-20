#include <SFML/Graphics.hpp>
#include <queue>
#include <stack>
#include <vector>
#include <iostream>
#include <fstream>
#include <climits>
#include <cmath>
#include <chrono>
#include <functional>

#define WIDTH 800
#define HEIGHT 800
#define ROWS 20
#define COLS 20
#define CELL_SIZE (WIDTH / COLS)

enum Algorithm
{
    DIJKSTRA,
    BFS,
    DFS,
    ASTAR
};
enum AppState
{
    IDLE,
    RUNNING,
    FINISHED
};

struct Node
{
    int x, y;
    bool isWall;
    bool visited;
    float distance;
    float heuristic;
    Node *parent;

    Node(int x = 0, int y = 0)
        : x(x), y(y), isWall(false), visited(false),
          distance(INT_MAX), heuristic(0), parent(nullptr) {}

    float getCost() const { return distance + heuristic; }
};

// ── grid helpers ────────────────────────────────────────────────────────────

std::vector<std::vector<Node>> createGrid(int rows, int cols)
{
    std::vector<std::vector<Node>> grid(rows, std::vector<Node>(cols));
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            grid[i][j] = Node(i, j);
    return grid;
}

std::vector<Node *> getNeighbors(Node &node, std::vector<std::vector<Node>> &grid)
{
    std::vector<Node *> neighbors;
    int x = node.x, y = node.y;
    if (x > 0)
        neighbors.push_back(&grid[x - 1][y]);
    if (x < ROWS - 1)
        neighbors.push_back(&grid[x + 1][y]);
    if (y > 0)
        neighbors.push_back(&grid[x][y - 1]);
    if (y < COLS - 1)
        neighbors.push_back(&grid[x][y + 1]);
    return neighbors;
}

void resetGrid(std::vector<std::vector<Node>> &grid, Node *start)
{
    for (auto &row : grid)
        for (auto &node : row)
        {
            node.visited = false;
            node.parent = nullptr;
            node.distance = INT_MAX;
            node.heuristic = 0;
        }
    start->distance = 0;
}

// ── drawing ─────────────────────────────────────────────────────────────────

void drawGrid(sf::RenderWindow &window,
              std::vector<std::vector<Node>> &grid,
              Node *start, Node *end)
{
    for (auto &row : grid)
    {
        for (auto &node : row)
        {
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1.f, CELL_SIZE - 1.f));
            cell.setPosition(node.y * CELL_SIZE + 0.5f, node.x * CELL_SIZE + 0.5f);
            if (node.isWall)
                cell.setFillColor(sf::Color::Black);
            else if (&node == start)
                cell.setFillColor(sf::Color::Green);
            else if (&node == end)
                cell.setFillColor(sf::Color::Red);
            else if (node.visited)
                cell.setFillColor(sf::Color::Cyan);
            else
                cell.setFillColor(sf::Color::White);
            window.draw(cell);
        }
    }
}

void drawPath(sf::RenderWindow &window, Node *end)
{
    Node *current = end->parent; // skip end node itself so red stays visible
    while (current && current->parent)
    {
        sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1.f, CELL_SIZE - 1.f));
        cell.setPosition(current->y * CELL_SIZE + 0.5f, current->x * CELL_SIZE + 0.5f);
        cell.setFillColor(sf::Color::Yellow);
        window.draw(cell);
        current = current->parent;
    }
}

// ── step-based algorithm state ───────────────────────────────────────────────

struct AlgoState
{
    Algorithm algo;

    // BFS / DFS containers
    std::queue<Node *> bfsQueue;
    std::stack<Node *> dfsStack;

    // Dijkstra / A* priority queue
    using PQComp = std::function<bool(Node *, Node *)>;
    std::priority_queue<Node *, std::vector<Node *>, PQComp> pq{
        [](Node *a, Node *b)
        { return a->getCost() > b->getCost(); }};

    Node *endNode = nullptr;
    bool found = false;

    void init(Algorithm a,
              std::vector<std::vector<Node>> &grid,
              Node *start, Node *end)
    {
        algo = a;
        endNode = end;
        found = false;

        // clear containers
        bfsQueue = std::queue<Node *>{};
        dfsStack = std::stack<Node *>{};
        pq = std::priority_queue<Node *, std::vector<Node *>, PQComp>(
            [](Node *a, Node *b)
            { return a->getCost() > b->getCost(); });

        if (algo == BFS)
        {
            start->visited = true;
            bfsQueue.push(start);
        }
        else if (algo == DFS)
        {
            start->visited = true;
            dfsStack.push(start);
        }
        else
        { // DIJKSTRA or ASTAR
            start->distance = 0;
            start->heuristic = (algo == ASTAR)
                                   ? std::abs(start->x - end->x) + std::abs(start->y - end->y)
                                   : 0;
            pq.push(start);
        }
    }

    // Returns true while still running, false when finished
    bool step(std::vector<std::vector<Node>> &grid)
    {
        if (found)
            return false;

        if (algo == BFS)
        {
            if (bfsQueue.empty())
                return false;
            Node *cur = bfsQueue.front();
            bfsQueue.pop();
            if (cur == endNode)
            {
                found = true;
                return false;
            }
            for (Node *nb : getNeighbors(*cur, grid))
            {
                if (nb->visited || nb->isWall)
                    continue;
                nb->visited = true;
                nb->parent = cur;
                if (nb == endNode)
                {
                    found = true;
                    return false;
                }
                bfsQueue.push(nb);
            }
        }
        else if (algo == DFS)
        {
            if (dfsStack.empty())
                return false;
            Node *cur = dfsStack.top();
            dfsStack.pop();
            if (cur == endNode)
            {
                found = true;
                return false;
            }
            for (Node *nb : getNeighbors(*cur, grid))
            {
                if (nb->visited || nb->isWall)
                    continue;
                nb->visited = true;
                nb->parent = cur;
                if (nb == endNode)
                {
                    found = true;
                    return false;
                }
                dfsStack.push(nb);
            }
        }
        else
        { // DIJKSTRA / ASTAR
            while (!pq.empty() && pq.top()->visited)
                pq.pop(); // skip stale entries
            if (pq.empty())
                return false;
            Node *cur = pq.top();
            pq.pop();
            if (cur == endNode)
            {
                found = true;
                return false;
            }
            cur->visited = true;
            for (Node *nb : getNeighbors(*cur, grid))
            {
                if (nb->visited || nb->isWall)
                    continue;
                float nd = cur->distance + 1;
                if (nd < nb->distance)
                {
                    nb->distance = nd;
                    nb->heuristic = (algo == ASTAR)
                                        ? std::abs(nb->x - endNode->x) + std::abs(nb->y - endNode->y)
                                        : 0;
                    nb->parent = cur;
                    pq.push(nb);
                }
            }
        }
        return true; // still running
    }
};

// ── file loading ─────────────────────────────────────────────────────────────

void readInputFromFile(std::vector<std::vector<Node>> &grid)
{
    std::ifstream inputFile("input4.txt");
    if (!inputFile)
    {
        // default: clear + border walls
        for (int i = 0; i < ROWS; ++i)
            for (int j = 0; j < COLS; ++j)
                grid[i][j].isWall = (i == 0 || i == ROWS - 1 || j == 0 || j == COLS - 1);
        return;
    }
    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j)
        {
            int v;
            inputFile >> v;
            grid[i][j].isWall = (v == 0);
        }
}

// ── main ─────────────────────────────────────────────────────────────────────

int main()
{
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Maze Solver");
    window.setFramerateLimit(60);

    std::vector<std::vector<Node>> grid = createGrid(ROWS, COLS);
    Node *start = &grid[1][1];
    Node *end = &grid[ROWS - 2][COLS - 2];

    readInputFromFile(grid);

    // Make sure start/end are not walls
    start->isWall = false;
    end->isWall = false;
    start->distance = 0;

    Algorithm algorithm = ASTAR;
    AppState appState = IDLE;
    AlgoState algoState;

    sf::Clock stepClock;
    const sf::Time STEP_INTERVAL = sf::milliseconds(10);

    std::cout << "ENTER = start | R = reset | Left-click = toggle wall\n";
    std::cout << "Keys 1=Dijkstra  2=BFS  3=DFS  4=A*\n";

    while (window.isOpen())
    {
        // ── event handling ──────────────────────────────────────────────────
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed)
            {
                switch (event.key.code)
                {
                case sf::Keyboard::Num1:
                    algorithm = DIJKSTRA;
                    std::cout << "Dijkstra selected\n";
                    break;
                case sf::Keyboard::Num2:
                    algorithm = BFS;
                    std::cout << "BFS selected\n";
                    break;
                case sf::Keyboard::Num3:
                    algorithm = DFS;
                    std::cout << "DFS selected\n";
                    break;
                case sf::Keyboard::Num4:
                    algorithm = ASTAR;
                    std::cout << "A* selected\n";
                    break;
                case sf::Keyboard::Enter:
                    if (appState == IDLE)
                    {
                        resetGrid(grid, start);
                        algoState.init(algorithm, grid, start, end);
                        appState = RUNNING;
                        stepClock.restart();
                        std::cout << "Algorithm started.\n";
                    }
                    break;
                case sf::Keyboard::R:
                    resetGrid(grid, start);
                    appState = IDLE;
                    std::cout << "Reset.\n";
                    break;
                default:
                    break;
                }
            }

            // Toggle walls only when idle
            if (event.type == sf::Event::MouseButtonPressed && appState == IDLE)
            {
                int row = event.mouseButton.y / CELL_SIZE;
                int col = event.mouseButton.x / CELL_SIZE;
                if (row >= 0 && row < ROWS && col >= 0 && col < COLS)
                {
                    Node *n = &grid[row][col];
                    if (n != start && n != end)
                        n->isWall = !n->isWall;
                }
            }
        }

        // ── algorithm stepping (non-blocking) ───────────────────────────────
        if (appState == RUNNING)
        {
            int stepsPerFrame = 3;
            for (int i = 0; i < stepsPerFrame && appState == RUNNING; ++i)
            {
                bool stillRunning = algoState.step(grid);
                if (!stillRunning)
                {
                    appState = FINISHED;
                    std::cout << "Algorithm finished. "
                              << (algoState.found ? "Path found!" : "No path found.") << "\n";
                }
            }
        }

        // ── render ───────────────────────────────────────────────────────────
        window.clear(sf::Color(40, 40, 40));
        drawGrid(window, grid, start, end);
        if (appState == FINISHED)
            drawPath(window, end);
        window.display();
    }

    return 0;
}