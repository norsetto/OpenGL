#pragma once

#include "glad.h"
#include "image.hpp"
#include "config.h"

#include <glm/glm.hpp>

#include <map>
#include <string>
#include <cfloat>
#include <assert.h>
#include <vector>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

class Object {

public:

  void load(const std::string &filename, bool compute_bounding_box = true, bool compute_TB = false);
  void render(GLsizei num_instances = 1);
  void free();

  //Getters
  float scale();
  glm::vec3 min();
  glm::vec3 max();
    
private:

  //Buffers for rendering
  GLuint num_meshes;
  GLuint *vao;
  std::vector<GLuint> buffers;

  //We do indexed rendering
  GLuint *num_indices;

  //Material properties
  struct MATERIALS_BLOCK {
    glm::vec4 diffuse;
    glm::vec4 ambient;
    glm::vec4 specular;
    //      glm::vec4 emissive;
    glm::vec4 auxilary;
  } *materials;
  GLuint *material_buffer;
  GLuint *textureDiffId;
  GLuint *textureNormId;
  GLuint *textureSpecId;

  //Bounding box
  struct BOUNDING_BOX {
    glm::vec3 min;
    glm::vec3 max;
    float scale;
  }* bb = NULL;
  
};

void Object::load(const std::string &filename, bool compute_bounding_box, bool compute_TB) {
  Assimp::Importer importer;
  unsigned int aiFlags = aiProcess_GenSmoothNormals |
    aiProcess_JoinIdenticalVertices    |
    aiProcess_ImproveCacheLocality     |
    aiProcess_RemoveRedundantMaterials |
    aiProcess_Triangulate              |
    aiProcess_GenUVCoords              |
    aiProcess_SortByPType              |
    aiProcess_FindDegenerates          |
    aiProcess_FindInvalidData          |
    aiProcess_ValidateDataStructure;

  if (compute_TB) aiFlags = aiFlags | aiProcess_CalcTangentSpace;

  const aiScene* scene = importer.ReadFile(filename, aiFlags);
    
  assert(scene);
  assert(scene->HasMeshes());

  std::map<std::string, GLuint> textureDiffIdMap;	
  std::map<std::string, GLuint> textureNormIdMap;	
  std::map<std::string, GLuint> textureSpecIdMap;	

  for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
    int texIndex = 0;
    aiString path;
 
    aiReturn texFound = scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
    while (texFound == AI_SUCCESS) {
      textureDiffIdMap[path.data] = 0;
      texIndex++;
      texFound = scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
    }

    texIndex = 0;
 
    texFound = scene->mMaterials[i]->GetTexture(aiTextureType_NORMALS, texIndex, &path);
    while (texFound == AI_SUCCESS) {
      textureNormIdMap[path.data] = 0;
      texIndex++;
      texFound = scene->mMaterials[i]->GetTexture(aiTextureType_NORMALS, texIndex, &path);
    }

    texIndex = 0;
 
    texFound = scene->mMaterials[i]->GetTexture(aiTextureType_SPECULAR, texIndex, &path);
    while (texFound == AI_SUCCESS) {
      textureSpecIdMap[path.data] = 0;
      texIndex++;
      texFound = scene->mMaterials[i]->GetTexture(aiTextureType_SPECULAR, texIndex, &path);
    }
  }

  std::map<std::string, GLuint>::iterator itr;
  for (itr = textureDiffIdMap.begin(); itr != textureDiffIdMap.end(); itr++)
    {
      GLuint texture_id;
      Image image(texture_id, (*itr).first);
      (*itr).second = texture_id;
    }
  for (itr = textureNormIdMap.begin(); itr != textureNormIdMap.end(); itr++)
    {
      GLuint texture_id;
      Image image(texture_id, (*itr).first);
      (*itr).second = texture_id;
    }
  for (itr = textureSpecIdMap.begin(); itr != textureSpecIdMap.end(); itr++)
    {
      GLuint texture_id;
      Image image(texture_id, (*itr).first);
      (*itr).second = texture_id;
    }
    
  GLuint buffer;
  num_meshes = scene->mNumMeshes;
  vao = new GLuint [num_meshes];
  glGenVertexArrays(num_meshes, vao);
  num_indices = new GLuint [num_meshes];
  material_buffer = new GLuint [num_meshes];
  glGenBuffers(num_meshes, material_buffer);
  materials = new MATERIALS_BLOCK [num_meshes];
  textureDiffId = new GLuint [num_meshes];
  textureNormId = new GLuint [num_meshes];
  textureSpecId = new GLuint [num_meshes];

  if (compute_bounding_box) {
    bb = new BOUNDING_BOX;
    bb->min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    bb->max = glm::vec3(FLT_MIN, FLT_MIN, FLT_MIN);
    bb->scale = 1.0f;
  }
    
  for (unsigned int i = 0; i < num_meshes; i++) {

    const aiMesh* mesh = scene->mMeshes[i];

    num_indices[i] = mesh->mNumFaces * 3;
    unsigned int* indices = new unsigned int [num_indices[i]];
    unsigned int faceIndex = 0;

    for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
      const aiFace* face = &mesh->mFaces[j];
 
      memcpy(&indices[faceIndex], face->mIndices, 3 * sizeof(unsigned int));
      faceIndex += 3;
    }
     
    glBindVertexArray(vao[i]);
      
    if (mesh->HasPositions()) {
      glGenBuffers(1, &buffer);
      buffers.push_back(buffer);
      glBindBuffer(GL_ARRAY_BUFFER, buffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh->mNumVertices, mesh->mVertices, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
      glEnableVertexAttribArray(0);

      if (compute_bounding_box) {
	  for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
	    bb->min.x = gl_min(bb->min.x, mesh->mVertices[j].x);
	    bb->min.y = gl_min(bb->min.y, mesh->mVertices[j].y);
	    bb->min.z = gl_min(bb->min.z, mesh->mVertices[j].z);

	    bb->max.x = gl_max(bb->max.x, mesh->mVertices[j].x);
	    bb->max.y = gl_max(bb->max.y, mesh->mVertices[j].y);
	    bb->max.z = gl_max(bb->max.z, mesh->mVertices[j].z);
	  }
      }
    }

    if (mesh->HasNormals()) {
      glGenBuffers(1, &buffer);
      buffers.push_back(buffer);
      glBindBuffer(GL_ARRAY_BUFFER, buffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh->mNumVertices, mesh->mNormals, GL_STATIC_DRAW);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    if (mesh->HasTextureCoords(0)) {
      float *texCoords = new float [2*mesh->mNumVertices];
      for (unsigned int k = 0; k < mesh->mNumVertices; k++) {
	  
	texCoords[k*2]   = mesh->mTextureCoords[0][k].x;
	texCoords[k*2+1] = mesh->mTextureCoords[0][k].y; 
 
      }
      glGenBuffers(1, &buffer);
      buffers.push_back(buffer);
      glBindBuffer(GL_ARRAY_BUFFER, buffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*mesh->mNumVertices, texCoords, GL_STATIC_DRAW);
      glEnableVertexAttribArray(2);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
      delete [] texCoords;
    }

    if (mesh->HasTangentsAndBitangents()) {
      glGenBuffers(1, &buffer);
      buffers.push_back(buffer);
      glBindBuffer(GL_ARRAY_BUFFER, buffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh->mNumVertices, mesh->mTangents, GL_STATIC_DRAW);
      glEnableVertexAttribArray(3);
      glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
      glGenBuffers(1, &buffer);
      buffers.push_back(buffer);
      glBindBuffer(GL_ARRAY_BUFFER, buffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh->mNumVertices, mesh->mBitangents, GL_STATIC_DRAW);
      glEnableVertexAttribArray(4);
      glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glGenBuffers(1, &buffer);
    buffers.push_back(buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices[i]*sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
      
    glBindVertexArray(0);
      
    delete [] indices;
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    aiMaterial *mtl = scene->mMaterials[mesh->mMaterialIndex];
 
    aiString texPath;
    if(AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, 0, &texPath))
      {
	materials[i].auxilary[1] = 1.0f;
	textureDiffId[i] = textureDiffIdMap[texPath.data];
	if(AI_SUCCESS == mtl->GetTexture(aiTextureType_NORMALS, 0, &texPath))
	  {
	    materials[i].auxilary[1] = 2.0f;
	    textureNormId[i] = textureNormIdMap[texPath.data];
	    if(AI_SUCCESS == mtl->GetTexture(aiTextureType_SPECULAR, 0, &texPath))
	      {
		materials[i].auxilary[1] = 3.0f;
		textureSpecId[i] = textureSpecIdMap[texPath.data];
	      }
	    else
	      textureSpecId[i] = 0;
	  } else
	  {
	    textureNormId[i] = 0;
	    if(AI_SUCCESS == mtl->GetTexture(aiTextureType_SPECULAR, 0, &texPath))
	      {
		materials[i].auxilary[1] = 4.0f;
		textureSpecId[i] = textureSpecIdMap[texPath.data];
	      }
	    else
	      textureSpecId[i] = 0;
	  }
      }
    else
      {
	materials[i].auxilary[1] = 0.0f;
	textureDiffId[i] = 0;
	textureNormId[i] = 0;
	textureSpecId[i] = 0;
      }
      
    aiColor4D diffuse;
    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
      materials[i].diffuse = glm::vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
    else
      materials[i].diffuse = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
    
    aiColor4D ambient;
    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
      materials[i].ambient = glm::vec4(ambient.r, ambient.g, ambient.b, ambient.a);
    else
      materials[i].ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
 
    aiColor4D specular;
    if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
      materials[i].specular = glm::vec4(specular.r, specular.g, specular.b, specular.a);
    else
      materials[i].specular = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    /*    
	  aiColor4D emissive;
	  if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emissive))
	  materials[i].emissive = glm::vec4(emissive.r, emissive.g, emissive.b, emissive.a);
	  else
	  materials[i].emissive = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    */
    float shininess = 0.0;
    unsigned int max;
    aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
    materials[i].auxilary[0] = shininess;

    /* TODO roughness should be mesh dependant */
    materials[i].auxilary[2] = 0.5f;

    glBindBufferBase(GL_UNIFORM_BUFFER, 1, material_buffer[i]);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MATERIALS_BLOCK), (void *)(&materials[i]), GL_STATIC_DRAW);

  }
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  if (compute_bounding_box) {
    float tmp = bb->max.x - bb->min.x;
    tmp = bb->max.y - bb->min.y > tmp?bb->max.y - bb->min.y:tmp;
    tmp = bb->max.z - bb->min.z > tmp?bb->max.z - bb->min.z:tmp;
    bb->scale = 1.f / tmp;
  }
}


float Object::scale()
{
  return bb->scale;
}

glm::vec3 Object::max()
{
  return bb->max;
}
  
glm::vec3 Object::min()
{
  return bb->min;
}

void Object::render(GLsizei num_instances) {
  for (unsigned int i = 0; i < num_meshes; i++) {
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, material_buffer[i], 0, sizeof(MATERIALS_BLOCK));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureDiffId[i]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureNormId[i]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureSpecId[i]);
    glBindVertexArray(vao[i]);
    glDrawElementsInstanced(GL_TRIANGLES, num_indices[i], GL_UNSIGNED_INT, NULL, num_instances);
  }
  glBindVertexArray(0);
  glActiveTexture(GL_TEXTURE0);
}

void Object::free() {
  if (vao) {
    glDeleteVertexArrays(num_meshes, vao);
    delete[] vao;
    vao = NULL;
  }
  if (num_indices) {
    delete[] num_indices;
    num_indices = NULL;
  }
  glDeleteBuffers(num_meshes, material_buffer);
  glDeleteBuffers(buffers.size(), buffers.data());
  glDeleteTextures(num_meshes, textureDiffId);
  glDeleteTextures(num_meshes, textureNormId);
  glDeleteTextures(num_meshes, textureSpecId);
  buffers.clear();
  delete[] materials;
  materials = NULL;
  delete[] material_buffer;
  material_buffer = NULL;
  delete[] textureDiffId;
  textureDiffId = NULL;
  delete[] textureNormId;
  textureNormId = NULL;
  delete[] textureSpecId;
  textureSpecId = NULL;
  num_meshes = 0;
  if (bb != NULL) {
    delete bb;
    bb = NULL;
  }
}

