#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <string.h>

#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

float brickl = 0.1;
float brickw= 0.2;
float mirrorl = 0.2;
float mirrorw = 0.1;
float p = 0.18f;
float brickspeed = 0.01;
float bucketwidth = 0.4;
struct VAO {
  GLuint VertexArrayID;
  GLuint VertexBuffer;
  GLuint ColorBuffer;

  GLenum PrimitiveMode;
  GLenum FillMode;
  int NumVertices;
};
typedef struct VAO VAO;
struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
  struct VAO* vao = new struct VAO;
  vao->PrimitiveMode = primitive_mode;
  vao->NumVertices = numVertices;
  vao->FillMode = fill_mode;

    // Create Vertex Array Object
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
  }

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
  struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
  {
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
      color_buffer_data [3*i] = red;
      color_buffer_data [3*i + 1] = green;
      color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
  }

/* Render the VBOs handled by VAO */
  void draw3DObject (struct VAO* vao)
  {
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
  }

/**************************
 * Customizable functions *
 **************************/

/* Executed when a regular key is pressed */


// /* Executed when a regular key is released */

/* Executed when a mouse button 'button' is put into state 'state'
 at screen position ('x', 'y')
 */

/* Executed when the mouse moves to position ('x', 'y') */
  void mouseMotion (int x, int y)
  {
  }


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
  void reshapeWindow (int width, int height)
  {
   GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
   glViewport (0, 0, (GLsizei) width, (GLsizei) height);

	// set the projection matrix as perspective/ortho
	// Store the projection matrix in a variable for future use

    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) width / (GLfloat) height, 0.1f, 500.0f);

    // Ortho projection for 2D views
   Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
 }

//VAO *triangle, *rectangle;

 VAO* createBrick (int color)
 {
  // GL3 accepts only Triangles. Quads are not supported 
   static const GLfloat vertex_buffer_data [] = {
    -brickl,-brickw,0, // vertex 1
    brickl,-brickw,0, // vertex 2
    brickl, brickw,0, // vertex 3

    brickl, brickw,0, // vertex 3
    -brickl, brickw,0, // vertex 4
    -brickl,-brickw,0  // vertex 1
  };
 //   cout << "hi" << c;
  int x,y,z;
  switch (color){
  	case 0: // black brick
    x= 0;y=0;z=0;
    break;

  	case 1: // green brick
    x=1;y=0;z=0;
    break;

  	case 2: // green brick
    x=0;y=1;z=0;
    break;
  }
  const GLfloat color_buffer_data [] = {
    x,y,z,
    x,y,z,
    x,y,z,
    x,y,z,
    x,y,z,
    x,y,z // color 1
  };


  // create3DObject creates and returns a handle to a VAO that can be used later
  return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
VAO* createBucket(int color)
{

	static const GLfloat vertex_buffer_data [] = {
    -bucketwidth,-0.1,0, // vertex 1
    bucketwidth,-0.1,0, // vertex 2
    bucketwidth, 0.1,0, // vertex 3

    bucketwidth, 0.1,0, // vertex 3
    -bucketwidth, 0.1,0, // vertex 4
    -bucketwidth,-0.1,0  // vertex 1
  };
  int x,y;
  switch(color){
	case 1: //red bucket
  x=1;y=0;
  break;
	case 2: //green bucket
  x=0;y=1;
  break;
}

const GLfloat color_buffer_data [] = {
  x,y,0,
  x,y,0,
  x,y,0,
  x,y,0,
  x,y,0,
  x,y,0
};
return create3DObject(GL_TRIANGLES, 12, vertex_buffer_data, color_buffer_data, GL_FILL);
};
VAO* createMirror()
{
  static const GLfloat vertex_buffer_data [] = {
    -mirrorl,-mirrorw,0, // vertex 1
    mirrorl,-mirrorw,0, // vertex 2
    mirrorl, mirrorw,0, // vertex 3

    mirrorl, mirrorw,0, // vertex 3
    -mirrorl, mirrorw,0, // vertex 4
    -mirrorl,-mirrorw,0  // vertex 1
  };	
  const GLfloat color_buffer_data [] = {
    0.5,0.5,0.5,
    0.5,0.5,0.5,
    0.5,0.5,0.5,
    0.5,0.5,0.5,
    0.5,0.5,0.5,
    0.5,0.5,0.5 // color 1
  };
  return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}


VAO* createCanon()
{
  const GLfloat vertex_buffer_data [] = {
    //z=0 since we are working in 2D
    -0.3,-0.25,0, // vertex 1
    0.3,-0.25,0, // vertex 2
    0.3, 0.25,0, // vertex 3

    0.3, 0.25,0, // vertex 3
    -0.3, 0.25,0, // vertex 4
    -0.3,-0.25,0,  // vertex 1

    0.3,-0.125,0, // vertex 1
    0.6,-0.125,0, // vertex 2
    0.6, 0.125,0, // vertex 3

    0.6, 0.125,0, // vertex 3
    0.3, 0.125,0, // vertex 4
    0.3,-0.125,0  // vertex 1
    //v3,v2,v6
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0,  // color 1
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };
// create3DObject creates and returns a handle to a VAO that can be used later
  return create3DObject(GL_TRIANGLES, 12, vertex_buffer_data, color_buffer_data, GL_FILL);
};
VAO* DrawLine(float bx, float by, float x, float y)
{
   const GLfloat vertex_buffer_data [] = {
    bx,by,0, // vertex 1
    x,y,0
  };
  const GLfloat color_buffer_data [] = {
    0,0,1,
    0,0,1,
  };
  Matrices.model = glm::mat4(1.0f);

  VAO* point = create3DObject(GL_LINES, 2, vertex_buffer_data, color_buffer_data, GL_LINE);
    glm::mat4 transform = glm::translate (glm::vec3(0, 0, 0.0f)); // glTranslatef
    Matrices.model *= transform;
    glm::mat4 MVP;
    glm::mat4 VP = Matrices.projection * Matrices.view;
  	MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(point);
  };


// VAO* createRectangle ()
// {
//   // GL3 accepts only Triangles. Quads are not supported static
//   const GLfloat vertex_buffer_data [] = {
//     // Fixing the size of the bric
//     -1.2,-1,0, // vertex 1
//     1.2,-1,0, // vertex 2
//     1.2, 1,0, // vertex 3

//     1.2, 1,0, // vertex 3
//     -1.2, 1,0, // vertex 4
//     -1.2,-1,0  // vertex 1
//   };

//   static const GLfloat color_buffer_data [] = {
//     1,0,0, // color 1
//     0,0,1, // color 2
//     0,1,0, // color 3

//     0,1,0, // color 3
//     0.3,0.3,0.3, // color 4
//     1,0,0  // color 1
//   };

//   // create3DObject creates and returns a handle to a VAO that can be used later
//   return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
// }


  class Movable {
  public:
    VAO *obj;
    float x;
    float y;
    float angle;

    void place(float x, float y){
    	this->x = x;
      this->y = y;
      this->angle = 0;
    }
    void rotate(float angle)
    {
     this->angle += angle;

   }
   void draw(){

    Matrices.model = glm::mat4(1.0f);

  /* Render your scene */

  glm::mat4 transform = glm::translate (glm::vec3(this->x, this->y, 0.0f)); // glTranslatef
  glm::mat4 rotate = glm::rotate((float)(this->angle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (transform * rotate);

  glm::mat4 MVP;

  glm::mat4 VP = Matrices.projection * Matrices.view;
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(this->obj);

}
};
class Brick: public Movable {
public:
  int color;
  void first(){
//	   	VAO *t;
      int color = rand()%3;
      this->color = color;
  	this->obj = createBrick(color);
  }
  void destroy(){
    this->x = x - 100;

  }
};
class Bucket: public Movable{
public:
	int color;
	void first(int x){
		this->color = x;
		this->obj = createBucket(this->color);
	}
};
class Canon: public Movable{
public:
  void first()
  {
    this->obj = createCanon();
  }
};
class Mirror: public Movable{
public:
	void first()
	{
		this->obj = createMirror();
	}
};
Brick bricks[250]; // storing my bricks in an array
Bucket buckets[2];
Canon canon;
Mirror mirrors[3];

class Game{
public:
  int bcount=0;
  int mcount=0;
  int score=0;
  void addBrick(float x, float y){
    bricks[bcount]=Brick();
    bricks[bcount].place(x,y);
    bricks[bcount].first();
    bcount++;
  }
  void addMirror(float x, float y, float a){
   mirrors[mcount]= Mirror();
mirrors[mcount].first();
mirrors[mcount].place( x,y);
mirrors[mcount].rotate(a);
    mcount++;
  }
  void tick()
  {
    //check collision
    //update score
    //end game
  }
  void end(){
    exit(0);
  }
};
Game game;

// Creates the triangle object used in this sample code
// void createTriangle ()
// {
//   /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

//   /* Define vertex array as used in glBegin (GL_TRIANGLES) */
//   static const GLfloat vertex_buffer_data [] = {
//     0, 1,0, // vertex 0
//     -1,-1,0, // vertex 1
//     1,-1,0, // vertex 2
//   };

//   static const GLfloat color_buffer_data [] = {
//     1,0,0, // color 0
//     0,1,0, // color 1
//     0,0,1, // color 2
//   };

//   // create3DObject creates and returns a handle to a VAO that can be used later
//   triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
// }



float camera_rotation_angle = 90;
/* Render the scene with openGL */
/* Edit this function according to your assignment */
bool should = false;
bool ref = false;
float shootx= 0;
float shooty = 0;
float shoota = 0;
void shouldShoot(float x,float y,float a){
  shootx=x;
  shooty=y;
  shoota=a;
  should=true;
}
bool checkLaserCollision(float x, float y, float a){
  // printf("%f %f %s\n", x,y,((x<3) && (x>-3) && (y<3) && (y>-3)) ? "true" : "false");
  // for(int i=0;i<game.bcount;)
  //   printf("%s %s %s %s\n", ()? "true" : "false"  , (x<(bricks[i].x+(brickl/2)))? "true" : "false"  , (y>(bricks[i].y-(brickw/2)))? "true" : "false"  , (y<(bricks[i].y+(brickw/2))) ? "true" : "false" ); 
  if ((x<=3.8) && (x>=-4) && (y<=3.8) && (y>=-3.8))
  {
    for(int i = 0; i<game.bcount; i++){
      float x1,x2,y1,y2;
      x1 = bricks[i].x-brickl/2;
      x2 = bricks[i].x+brickl/2;
      y1 = bricks[i].y-brickw/2;
      y2 = bricks[i].y+brickw/2;
      //for brick
      if( x>x1 && x<x2 && y>y1 && y<y2 ){
        bricks[i].destroy();
        if (bricks[i].color==0)
        {
          game.end();
        }
        return false;
      }
    }
    //for mirror
    for(int i = 0; i<game.mcount; i++){
      float x1,x2,y1,y2,an,xn,yn,A,B,C1,C2,l1,l2;
      an = mirrors[i].angle;
      xn =  mirrors[i].x;
      yn = mirrors[i].y;
      A = cos(an);
      B = -sin(an);
      C1 = -A*xn - B*yn;
      C2 = B*xn - A*yn;
      l1 = A*x + B*y + C1;
      l2 = -B*x + A*y + C2;
      if( abs(l1)<mirrorw && abs(l2)<mirrorl ){
        if(ref==false){
          ref = true;
          shouldShoot(x-p*cos(a*3.14/180),y-p*sin(a*3.14/180),90 -a+ mirrors[i].angle);
          return false;
        }else{
          ref=false;
          return false;
        }
      }
    }
    return true;
  }
  else
    return false;
}  
void shoot(float x, float y, float a){
  float bx=x;
  float by=y;
  while(checkLaserCollision(x,y,a)){
    x = x + p*cos(a*3.14/180);
    y = y + p*sin(a*3.14/180);
  }
  DrawLine(bx,by,x,y);
  if (should)
  {
    should=false;
    shoot(shootx,shooty,shoota);
  }
}

void hold(int key){

  //int mod = glutGetModifiers(); 
  //bool alt = (mod == GLUT_ACTIVE_ALT); 
  //bool ctrl =  (mod == GLUT_ACTIVE_CTRL); 
  bool ctrl = true;
  if(key!=264)
  switch(key){

    case 'g':
      buckets[0].x-=0.1;
      break;
    case 'j':
        buckets[1].x-=0.1;
      break;
    case 'h':
        buckets[0].x+=0.1;
      break;
    case 'k':
        buckets[1].x+=0.1;
      break;
    case 'a':
      canon.angle++;
      break;  
    case 'd':
      canon.angle--;
      break;
    case 'n':
      brickspeed+=0.01;
      break;
    case 'm' :
      brickspeed-=0.01;
      break;
    case 's' :
      canon.y+=0.1;
      break;
    case 'f' :
      canon.y-=0.1;
      break;
  }
}

bool held[256];
void holdRun(){
  for(int i =0 ; i< 265; i ++){
    if (held[i])
    {
      hold(i);
    }
  }
}
void Sprint( int x, int y, char *st)
{
  int l,i;
  l=strlen( st ); // see how many characters are in text string.
  glRasterPos2i( x, y); // location to start printing text
  for( i=0; i < l; i++) // loop until i is greater then l
  {
    glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, st[i]); // Print a character on the screen
  }
}
void checkBucket(int b){
  for(int i = 0; i<game.bcount; i++){
      float x1,x2,y1,y2;
      x1 = bricks[i].x-brickl/2;
      x2 = bricks[i].x+brickl/2;
      y1 = bricks[i].y-brickw/2;
      y2 = bricks[i].y+brickw/2;
      if(y1 <= -3.9){
        if(x1>buckets[b].x-bucketwidth && x2<buckets[b].x+bucketwidth ){
          bricks[i].destroy();          
          if(buckets[b].color == bricks[i].color){
            game.score++;
          }
        }
      }
    }
}

void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 static int prev;
 static int anotherprev;
  int now = glutGet(GLUT_ELAPSED_TIME); 
  int timeelapsed = now - prev;
  int anothertimeelapsed = now - anotherprev;
  if (timeelapsed > 5)
  {
    printf("Score: %d\n", game.score);
    prev = now;
      holdRun();
      checkBucket(0);
      checkBucket(1);
    for( unsigned int a = 0; a < game.bcount; a = a + 1 ){
      bricks[a].y = bricks[a].y-brickspeed;
    }
    if(held[' '])
      shoot(canon.x,canon.y,canon.angle);

    /* code */
  }
  if (anothertimeelapsed>10000)
  {
    for( int i=0; i< 5; i++ ){
      float r2 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/8.0f));
      game.addBrick(r2 - 4.0f, +4.0f);
}
  }
  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
//  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  
 Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
 glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  /* Render your scene */

  glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
  //glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  //glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  //Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  // draw3DObject(triangle);
  for( unsigned int a = 0; a < game.bcount; ++a ){
    bricks[a].draw();
  }
  buckets[0].draw();
  buckets[1].draw();
  canon.draw();
  for( int i = 0; i < game.mcount; i++){
    mirrors[i].draw();
  }
  //char a[]={"GAME PAUSED"};
//GLvoid *font = GLUT_BITMAP_TIMES_ROMAN_24;
  //glutBitmapString (font,&a);
		  // Matrices.model = glm::mat4(1.0f);

  // glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
  // //glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  // //MVP = VP * Matrices.model;
  // glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // // draw3DObject draws the VAO given to it using current MVP matrix
  // draw3DObject(rectangle);

  // // Swap the frame buffers
  glutSwapBuffers ();

  // // Increment angles
  // float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  //triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  //rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Executed when the program is idle (no I/O activity) */
void idle () {
    // OpenGL should never stop drawing
    // can draw the same scene or a modified scene
    draw (); // drawing same scene
  }
// void move(bool alt,int key){
// 	if(alt){
// 		if(key = )
// 	}
// }



void keyboardDown (unsigned char key, int x, int y)
{
	printf("%c\n", key);
    
    held[key] = true;
    switch (key) {
        case 'l':
        	buckets[0].x = buckets[0].x+1;
        // case 's':
        // case 'S':
        // 	y=y-10;
        // case 'Q':
        case 'q':
        case 27: //ESC
            exit (0);

        default:
            break;
    }
}

 void keyboardUp (unsigned char key, int x, int y)
{
    held[key] = false;
    switch (key) {
        case 'c':
        case 'C':
            break;
        case 'p':
        case 'P':
            break;
         case 'x':

             // do something
             break;
         default:
             break;
    }
}

// /* Executed when a special key is pressed */
// void keyboardSpecialDown (int key, int x, int y)
// {
// }

//  Executed when a special key is released 
// void keyboardSpecialUp (int key, int x, int y)
// {
// }


/* Initialise glut window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
  void initGLUT (int& argc, char** argv, int width, int height)
  {
    // Init glut
    glutInit (&argc, argv);

    // Init glut window
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitContextVersion (3, 3); // Init GL 3.3
    glutInitContextFlags (GLUT_CORE_PROFILE); // Use Core profile - older functions are deprecated
    glutInitWindowSize (width, height);
    glutCreateWindow ("Sample OpenGL3.3 Application");

    // Initialize GLEW, Needed in Core profile
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
      cout << "Error: Failed to initialise GLEW : "<< glewGetErrorString(err) << endl;
      exit (1);
    }

    // register glut callbacks
    glutKeyboardFunc (keyboardDown);
    glutKeyboardUpFunc (keyboardUp);

    // glutSpecialFunc (keyboardSpecialDown);
    // glutSpecialUpFunc (keyboardSpecialUp);

    // glutMouseFunc (mouseClick);
    // glutMotionFunc (mouseMotion);

    glutReshapeFunc (reshapeWindow);

    glutDisplayFunc (draw); // function to draw when active
    glutIdleFunc (idle); // function to draw when idle (no I/O activity)
    
    glutIgnoreKeyRepeat (true); // Ignore keys held down
  }

/* Process menu option 'op' */
  void menu(int op)
  {
    switch(op)
    {
      case 'Q':
      case 'q':
      exit(0);
    }
  }

  void addGLUTMenus ()
  {
    // create sub menus
    int subMenu = glutCreateMenu (menu);
    glutAddMenuEntry ("Do Nothing", 0);
    glutAddMenuEntry ("Really Quit", 'q');

    // create main "middle click" menu
    glutCreateMenu (menu);
    glutAddSubMenu ("Sub Menu", subMenu);
    glutAddMenuEntry ("Quit", 'q');
    glutAttachMenu (GLUT_MIDDLE_BUTTON);
  }


/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
  void initGL (int width, int height)
  {
	// Create the models
	// Create and compile our GLSL program from the shaders
   programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
   Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


   reshapeWindow (width, height);

	// Background color of the scene
	glClearColor (1.0f, 0.5f, 1.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}
int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;
 srand (time(NULL));

 initGLUT (argc, argv, width, height);

 addGLUTMenus ();

 initGL (width, height);
//		cout << brick.x;

 int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
 for( unsigned int a = 0; a < 15; a = a + 1 ){
  float r2 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/8.0f));
  game.addBrick(r2 - 4.0f, r2 -  4.0f);
}
	//keyboardDown()
buckets[0]=Bucket();
buckets[0].place(-3,-4);
buckets[0].first(1);
buckets[1]=Bucket();
buckets[1].place(2,-4);
buckets[1].first(2);
canon.first();
canon.place(-4,0);
canon.rotate(60.0f);
game.addMirror(-2,1,45);
game.addMirror(3,3,30);
game.addMirror(-3,-1,60);
glutMainLoop ();

return 0;
}
