/*
 * Maze Game (Build upon Maze Generator):
 *   Made by Jaden Peterson in 2016
 */

/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This program uses the Depth-First Search algorithm
 *   You can learn more about it at:
 *     https://en.wikipedia.org/wiki/Maze_generation_algorithm#Depth-first_search
 */

#include <cstring>
#include <string>
#include <vector>

#include <GL/glut.h>

// A macro for unused variables (to bypass those pesky G++ warnings)
#define UNUSED(param) (void)(param)

// Direction macros
#define UP    0
#define DOWN  1
#define LEFT  2
#define RIGHT 3

/*
 * Key Codes:
 *   0 - UP    ('w', GLUT_KEY_UP   )
 *   1 - DOWN  ('s', GLUT_KEY_DOWN )
 *   2 - LEFT  ('a', GLUT_KEY_LEFT )
 *   3 - RIGHT ('d', GLUT_KEY_RIGHT)
 */

bool key_states[4] = {false, false, false, false};

const int base_window_size = 615;

// These two values are in milliseconds
const int key_repeat = 125;
const int timer_speed = 20;

int window_size[2];

// These two values are in milliseconds
unsigned long int *exec_time = 0;
unsigned long int *last_move = new unsigned long int;

const std::string window_title = "Maze Game";

class Maze {
    public:
        // Both values (width and hight) MUST be odd
        // Or the maze will have an extra wall
        const int size[2] = {41, 41};

        int level = 1;
        int player_location[2];

        // Three radii for three axes
        float radiuses[3];

        std::vector< std::vector< int > > part_locations;

        void generate();
        void initialize();
        void print();
        void randomPoint(bool part);

        bool movePlayer(int direction);

    bool start_axis;
    bool start_side;

    // The coordinates for various parts of the path
    std::vector< std::vector< int > > path;

    /*
     * Structure of the maze vector:
     *                     |--> Filled in?
     *   Row --> Collumn --|
     *                     |--> Has been visited?
     */
    std::vector< std::vector< std::vector< bool > > > maze;

    bool randomMove(bool first_move);
};

Maze maze;

char* getWindowTitle(){
    const std::string new_title = window_title + " - Level " + std::to_string(maze.level);
    char *new_title_cstr = new char[new_title.length() + 1];

    strcpy(new_title_cstr, new_title.c_str());

    return (char*) new_title_cstr;
}

// Selects a random direction to go, appends it to the current path, and moves there
bool Maze::randomMove(bool first_move){
    int random_neighbor;

    // The deltas of the current coordinates for possible directions to go
    std::vector< std::vector< int > > unvisited_neighbors;

    for(int direction = 0; direction < 4; direction++){
        int possible_pmd[2] = {0, 0};

        if(direction == UP){
            possible_pmd[1] = -1;
        } else if(direction == DOWN){
            possible_pmd[1] = 1;
        } else if(direction == LEFT){
            possible_pmd[0] = -1;
        } else {
            possible_pmd[0] = 1;
        }

        // Make sure it isn't on the walls of the maps
        if(path.back()[0] + possible_pmd[0] * 2 > 0 &&
           path.back()[0] + possible_pmd[0] * 2 < size[0] - 1 &&
           path.back()[1] + possible_pmd[1] * 2 > 0 &&
           path.back()[1] + possible_pmd[1] * 2 < size[1] - 1){
            // Make sure it hasn't been visited yet
            if(!maze[path.back()[1] + possible_pmd[1] * 2]
                    [path.back()[0] + possible_pmd[0] * 2][1]){
                std::vector< int > possible_move_delta = {possible_pmd[0], possible_pmd[1]};

                unvisited_neighbors.push_back(possible_move_delta);
            }
        }
    }

    if(unvisited_neighbors.size() > 0){
        random_neighbor = rand() % unvisited_neighbors.size();

        for(int a = 0; a < !first_move + 1; a++){
            std::vector< int > new_location;

            new_location.push_back(path.back()[0] + unvisited_neighbors[random_neighbor][0]);
            new_location.push_back(path.back()[1] + unvisited_neighbors[random_neighbor][1]);

            path.push_back(new_location);

            maze[path.back()[1]][path.back()[0]][0] = false;
            maze[path.back()[1]][path.back()[0]][1] = true;
        }

        return true;
    } else {
        return false;
    }
}

// The fun part ;)
void Maze::generate(){
    bool first_move = true;
    bool success = true;

    // While we haven't finished constructing the maze...
    while((int) path.size() > 1 - first_move){
        // If there are no ways to go, go back through the path
        // and find the first coordinate with no neighbors
        if(!success){
            path.pop_back();

            if(!first_move && path.size() > 2){
                path.pop_back();
            } else {
                break;
            }

            success = true;
        }

        // If the last movement was a success, keep moving
        while(success){
            success = randomMove(first_move);

            if(first_move){
                first_move = false;
            }
        }
    }
}

// Initialize the maze vector with a completely-filled grid with the size the user specified
void Maze::initialize(){
    for(int a = 0; a < size[1]; a++){
        for(int b = 0; b < size[0]; b++){
            bool is_border = false;

            // Is it a border?
            if(a == 0 || a == size[1] - 1 ||
               b == 0 || b == size[0] - 1){
                is_border = true;
            }

            std::vector< bool > new_cell = {true, is_border};

            // Must we create a new element,
            // or does one already exist in the vector for the current row?
            if(a + 1 > (int) maze.size()){
                std::vector< std::vector< bool > > new_row = {new_cell};

                maze.push_back(new_row);
            } else {
                maze[a].push_back(new_cell);
            }
        }
    }
}

bool Maze::movePlayer(int direction){
    int delta_x = 0;
    int delta_y = 0;

    bool exit_game = false;
    bool next_level = false;

    if(direction == UP){
        delta_y = 1;
    } else if(direction == DOWN){
        delta_y = -1;
    } else if(direction == LEFT){
        delta_x = -1;
    } else {
        delta_x = 1;
    }

    if(player_location[0] + delta_x < 0 ||
       player_location[0] + delta_x > size[0] ||
       player_location[1] + delta_y < 0 ||
       player_location[1] + delta_y > size[1]){
        exit_game = true;
    } else if(player_location[0] + delta_x == part_locations[1][0] &&
              player_location[1] + delta_y == part_locations[1][1]){
        next_level = true;
    } else {
        if(!maze[player_location[1] + delta_y][player_location[0] + delta_x][0]){
            player_location[0] += delta_x;
            player_location[1] += delta_y;
        } else {
            return false;
        }
    }

    if(exit_game){
        exit(0);
    } else if(next_level){
        maze.clear();
        part_locations.clear();
        path.clear();

        initialize();
        randomPoint(false);
        randomPoint(true);
        generate();

        level++;
        glutSetWindowTitle(getWindowTitle());
    }

    glutPostRedisplay();
    return true;
}

void Maze::print(){
    for(int a = 0; a < (int) maze.size(); a++){
        for(int b = 0; b < (int) maze[a].size(); b++){
            if(!maze[a][b][0]){
                glLoadIdentity();
                glTranslatef(b * 2 - radiuses[0], a * 2 - radiuses[1], -radiuses[2]);
                glColor3f(1.0f, 1.0f, 1.0f);

                bool is_player = false;

                for(int c = 0; c < 2; c++){
                    if(part_locations[c][0] == b &&
                       part_locations[c][1] == a){
                        if(!c){
                            glColor3f(0.0f, 1.0f, 0.0f);
                        } else {
                            glColor3f(1.0f, 0.3f, 0.3f);
                        }
                    }
                }

                if(player_location[0] == b &&
                   player_location[1] == a){
                    is_player = true;
                }

                glBegin(GL_TRIANGLE_STRIP);
                    glVertex2f(-1.0f,  1.0f);
                    glVertex2f( 1.0f,  1.0f);
                    glVertex2f(-1.0f, -1.0f);
                    glVertex2f( 1.0f, -1.0f);
                glEnd();

                if(is_player){
                    glColor3f(0.5f, 0.5f, 0.5f);

                    glBegin(GL_TRIANGLE_STRIP);
                        glVertex2d(-0.7f,  0.7f);
                        glVertex2d( 0.7f,  0.7f);
                        glVertex2d(-0.7f, -0.7f);
                        glVertex2d( 0.7f, -0.7f);
                    glEnd();
                }
            }
        }
    }
}

// Set a random point (start or end)
void Maze::randomPoint(bool part){
    bool axis;
    bool side;

    if(!part){
        axis = rand() % 2;
        side = rand() % 2;

        start_axis = axis;
        start_side = side;
    } else {
        bool done = false;

        while(!done){
            axis = rand() % 2;
            side = rand() % 2;

            if(axis != start_axis ||
               side != start_side){
                done = true;
            }
        }
    }

    std::vector< int > location = {0, 0};

    if(!side){
        location[!axis] = 0;
    } else {
        location[!axis] = size[!axis] - 1;
    }

    location[axis] = 2 * (rand() % ((size[axis] + 1) / 2 - 2)) + 1;
    part_locations.push_back(location);

    if(!part){
        path.push_back(location);

        std::copy(location.begin(), location.end(), player_location);
    }

    maze[location[1]][location[0]][0] = false;
    maze[location[1]][location[0]][1] = true;
}

void display(){
    if(exec_time - last_move >= key_repeat){
        for(int a = 0; a < 4; a++){
            if(key_states[a]){
                if(maze.movePlayer(a)){
                    last_move = exec_time;

                    break;
                }
            }
        }
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);

    maze.print();

    glutSwapBuffers();
}

void initGL(){
    glMatrixMode(GL_PROJECTION);
    gluPerspective(90.0f, (float) window_size[0] / (float) window_size[1], 0.0f, maze.radiuses[2]);
}

void initVariables(){
    last_move = (unsigned long int) 0;

    maze.radiuses[0] = maze.size[0] - 1.0f;
    maze.radiuses[1] = maze.size[1] - 1.0f;

    if(maze.size[0] > maze.size[1]){
        maze.radiuses[2] = maze.size[1] + 0.1f;

        window_size[0] = (double) maze.size[0] / (double) maze.size[1] * base_window_size;
        window_size[1] = base_window_size;
    } else {
        maze.radiuses[2] = maze.size[0] + 0.1f;

        window_size[0] = base_window_size;
        window_size[1] = (double) maze.size[0] / (double) maze.size[1] * base_window_size;
    }
}

void keyDown(unsigned char key, int x, int y){
    UNUSED(x);
    UNUSED(y);

    switch(key){
        case 'w':{
            key_states[0] = true;

            break;
        }

        case 's':{
            key_states[1] = true;

            break;
        }

        case 'a':{
            key_states[2] = true;

            break;
        }

        case 'd':{
            key_states[3] = true;

            break;
        }

        // Escape key
        case 27:
        case 'q':{
            exit(0);
        }
    }

    glutPostRedisplay();
}

void keyUp(unsigned char key, int x, int y){
    UNUSED(x);
    UNUSED(y);

    switch(key){
        case 'w':{
            key_states[0] = false;

            break;
        }

        case 's':{
            key_states[1] = false;

            break;
        }

        case 'a':{
            key_states[2] = false;

            break;
        }

        case 'd':{
            key_states[3] = false;
        }
    }
}

void reshape(int width, int height){
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(90.0f, (GLfloat) width / (GLfloat) height, 0.0f, maze.radiuses[2]);
}

void specialDown(int key, int x, int y){
    UNUSED(x);
    UNUSED(y);

    switch(key){
        case GLUT_KEY_UP:{
            key_states[0] = true;

            break;
        }

        case GLUT_KEY_DOWN:{
            key_states[1] = true;

            break;
        }

        case GLUT_KEY_LEFT:{
            key_states[2] = true;

            break;
        }

        case GLUT_KEY_RIGHT:{
            key_states[3] = true;
        }
    }

    glutPostRedisplay();
}

void specialUp(int key, int x, int y){
    UNUSED(x);
    UNUSED(y);

    switch(key){
        case GLUT_KEY_UP:{
            key_states[0] = false;

            break;
        }

        case GLUT_KEY_DOWN:{
            key_states[1] = false;

            break;
        }

        case GLUT_KEY_LEFT:{
            key_states[2] = false;

            break;
        }

        case GLUT_KEY_RIGHT:{
            key_states[3] = false;
        }
    }
}

// I have implemented my own key-repeat functionality
// It's not that great, but it runs every 125 milliseconds,
// and doesn't have that annoying delay most systems implement
void timer(int useless){
    UNUSED(useless);

    if(exec_time - last_move >= key_repeat || last_move == 0){
        if(key_states[0] ||
           key_states[1] ||
           key_states[2] ||
           key_states[3]){
            glutPostRedisplay();
        }
    }

    exec_time += timer_speed;
    glutTimerFunc(timer_speed, timer, 0);
}

int main(int argc, char **argv){
    srand(time(NULL));

    initVariables();

    maze.initialize();
    maze.randomPoint(false);
    maze.randomPoint(true);
    maze.generate();

    glutInit(&argc, argv);

    glutInitWindowSize(window_size[0], window_size[1]);
    glutCreateWindow(getWindowTitle());

    glutDisplayFunc(display);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);
    glutTimerFunc(timer_speed, timer, 0);

    glutIgnoreKeyRepeat(true);

    initGL();
    glutMainLoop();

    delete last_move;

    return 0;
}
