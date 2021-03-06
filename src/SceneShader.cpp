/*
 * SceneShader.cpp
 *
 *  Created on: Nov 17, 2015
 *      Author: acarocha
 */

#include "SceneShader.h"

static float PI = 3.14159265359;


SceneShader::SceneShader(): Shader()
{
	_programPlane = 0;
	_programMesh = 0;
	_planeVertexArray = -1;
	_planeVertexArray = -1;
	_mvUniform = -1;
	_projUniform = -1;
	_zTranslation = 1.0;
	_aspectRatio = 1.0;
	_xRot = 0.0;
	_yRot = 0.0;
	lightPosition = glm::vec3(0.5, 0.5, 0.5);

	setUpFragmentShaderProperties();
}

void SceneShader::setUpFragmentShaderProperties()
{
	b = askUserForSmallNumber("Enter the b(blue) value for the shader, must be between (0.0 and 1.0)");
	y = askUserForSmallNumber("Enter the y(yellow) value for the shader, must be between (0.0 and 1.0)");

	alpha = askUserForSmallNumber("Enter the alpha value for the shader, must be between (0.0 and 1.0)");
	beta = askUserForSmallNumber("Enter the beta value for the shader, must be between (0.0 and 1.0)");

	double rc, gc, bc;

	rc = askUserForSmallNumber("Enter the red color value for the kd in the shader, must be between (0.0 and 1.0)");
	gc = askUserForSmallNumber("Enter the green color value for the kd in the shader, must be between (0.0 and 1.0)");
	bc = askUserForSmallNumber("Enter the blue color value for the kd in the shader, must be between (0.0 and 1.0)");

	kd = glm::vec3(rc, gc, bc);

}


void SceneShader::readMesh( std::string filename )
{
	_mesh = trimesh::TriMesh::read(filename);

	_mesh->need_bbox();
	_mesh->need_faces();
	_mesh->need_normals();
	_mesh->need_bsphere();

	for(unsigned int i = 0; i < _mesh->faces.size(); i++)
	{
   	_triangleIndices.push_back(_mesh->faces[i][0]);
		_triangleIndices.push_back(_mesh->faces[i][1]);
		_triangleIndices.push_back(_mesh->faces[i][2]);
	}
}



void SceneShader::createVertexBuffer()
{
	//create plane geometry
	static const GLfloat quadData[] =
	{
                        -1.0f, 0.0f, -1.0f,
                        -1.0f, 0.0f, 1.0f,
                        1.0f, 0.0f, -1.0f,
                        1.0f, 0.0f, 1.0f,
	};

	//passing model attributes to the GPU
	//plane
	glGenVertexArrays(1, &_planeVertexArray);
	glBindVertexArray(_planeVertexArray);

	glGenBuffers(1, &_planeVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _planeVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof (quadData), quadData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	//read and create mesh geometry
	readMesh("./models/bunny.ply"); //normalized vertices coordinates

	//triangle mesh
	glGenVertexArrays(1, &_meshVertexArray);
	glBindVertexArray(_meshVertexArray);

	glGenBuffers(1, &_meshVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _meshVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER,  _mesh->vertices.size() * sizeof (trimesh::point), _mesh->vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	//TODO normals

        glGenBuffers(1, &_meshNormalBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, _meshNormalBuffer);
        glBufferData(GL_ARRAY_BUFFER,  _mesh->normals.size() * sizeof (trimesh::vec), _mesh->normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);



	glGenBuffers(1, &_meshIndicesBuffer );
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _meshIndicesBuffer );
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _triangleIndices.size()*sizeof(unsigned int), _triangleIndices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

}


void SceneShader::startup()
{
	_programPlane = compile_shaders("./shaders/plane.vert", "./shaders/plane.frag");

	_programMesh = compile_shaders("./shaders/mesh.vert", "./shaders/mesh.frag");

	_programLight = compile_shaders("./shaders/light.vert", "./shaders/light.frag");

	createVertexBuffer();

}

void SceneShader::renderPlane()
{
	glBindVertexArray(_planeVertexArray);

	glUseProgram(_programPlane);

	//scene matrices and camera setup
	glm::vec3 eye(0.0f,0.3f, 2.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::vec3 center(0.0f, 0.0f, 0.0f);

	_modelview = glm::lookAt( eye, center, up);

	glm::mat4 identity(1.0f);
	_projection = glm::perspective( 45.0f, _aspectRatio, 0.01f, 100.0f);

	glm::mat4 rotationX = glm::rotate(identity, _yRot  * PI/180.0f, glm::vec3(0.0f, 1.0f, 0.0f));

   _modelview *=  rotationX;

	//Uniform variables
	glUniformMatrix4fv(glGetUniformLocation(_programPlane, "modelview"), 1, GL_FALSE, glm::value_ptr(_modelview));
	glUniformMatrix4fv(glGetUniformLocation(_programPlane, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));

	glUniform3fv(glGetUniformLocation(_programPlane, "lightPosition"), 1, glm::value_ptr(lightPosition) );

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);
}

void SceneShader::renderMesh()
{
	glBindVertexArray(_meshVertexArray);

	glUseProgram(_programMesh);

	//scene matrices and camera setup
	glm::vec3 eye(0.0f, 0.3f, 2.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::vec3 center(0.0f, 0.0f, 0.0f);

	_modelview = glm::lookAt( eye, center, up);

	_projection = glm::perspective( 45.0f, _aspectRatio, 0.01f, 100.0f);

	glm::mat4 identity(1.0f);

	glm::mat4 rotationX = glm::rotate(identity, _yRot  * PI/180.0f, glm::vec3(0.0f, 1.0f, 0.0f));

   _modelview *=  rotationX;

	//uniform variables
	glUniformMatrix4fv(glGetUniformLocation(_programMesh, "modelview"), 1, GL_FALSE, glm::value_ptr(_modelview));
	glUniformMatrix4fv(glGetUniformLocation(_programMesh, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));


	glUniform1f(glGetUniformLocation(_programMesh, "b"), b);
	glUniform1f(glGetUniformLocation(_programMesh, "y"), y);


	glUniform1f(glGetUniformLocation(_programMesh, "alpha"), alpha);
	glUniform1f(glGetUniformLocation(_programMesh, "beta"), beta);

	glUniform3fv(glGetUniformLocation(_programMesh, "kd"), 1, glm::value_ptr(kd) );




	glUniform3fv(glGetUniformLocation(_programMesh, "lightPosition"), 1, glm::value_ptr(lightPosition) );

	glDrawElements( GL_TRIANGLES, _mesh->faces.size()*3, GL_UNSIGNED_INT, 0 );

	glBindVertexArray(0);
}


void SceneShader::renderLight()
{
	glUseProgram(_programLight);

	//scene matrices and camera setup
	glm::vec3 eye(0.0f, 0.3f, 2.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::vec3 center(0.0f, 0.0f, 0.0f);

	_modelview = glm::lookAt( eye, center, up);

	_projection = glm::perspective( 45.0f, _aspectRatio, 0.01f, 100.0f);

	glm::mat4 identity(1.0f);

	glm::mat4 rotationX = glm::rotate(identity, _yRot  * PI/180.0f, glm::vec3(0.0f, 1.0f, 0.0f));

   _modelview *=  rotationX;

	//uniform variables
	glUniformMatrix4fv(glGetUniformLocation(_programLight, "modelview"), 1, GL_FALSE, glm::value_ptr(_modelview));
	glUniformMatrix4fv(glGetUniformLocation(_programLight, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));

	glUniform3fv(glGetUniformLocation(_programLight, "lightPosition"), 1, glm::value_ptr(lightPosition) );

	glPointSize(30.0f);
	glDrawArrays( GL_POINTS, 0, 1);

}

void SceneShader::render()
{
	renderPlane();
	renderMesh();
	renderLight();
}

void SceneShader::setZTranslation(float z)
{
	_zTranslation = z;
}

void SceneShader::setAspectRatio(float ratio)
{
	_aspectRatio = ratio;
}

void SceneShader::setRotationX( float x )
{
	_xRot = x;
}

void SceneShader::setRotationY( float y )
{
	_yRot = y;
}

void SceneShader::shutdown()
{
	glDeleteBuffers(1, &_meshVertexBuffer);
	glDeleteBuffers(1, &_meshNormalBuffer);
	glDeleteBuffers(1, &_meshIndicesBuffer );
	glDeleteVertexArrays(1, &_meshVertexArray);
	glDeleteVertexArrays(1, &_planeVertexArray);
}

void SceneShader::updateLightPositionX(float x)
{
	lightPosition.x += x;
}

void SceneShader::updateLightPositionY(float y)
{
	lightPosition.y += y;
}

void SceneShader::updateLightPositionZ(float z)
{
	lightPosition.z += z;
}

float SceneShader::askUserForSmallNumber(std::string message)
{
	bool invalid = true;
	float x;
	while (invalid){
		std::cout << message << "\n";
		std::cin >> x;

		if (x < 0.0)
		{
			std::cout << "Number is less than 0, the number entered must be between 0 and 1";
			invalid = true;
		} else if ( x > 1.0)
		{
			std::cout << "Number is greater than 1, the number entered must be between 0 and 1";
			invalid = true;
		} else {
			invalid = false;
		}
	}
	return x;

}

SceneShader::~SceneShader()
{
	shutdown();
}
