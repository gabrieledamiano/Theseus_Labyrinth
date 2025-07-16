#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include "mesh.h"
#include "shader_m.h"
#include "animator.h"
#include "bone.h"
#include "assimp_glm_helpers.h" // Assumo che tu abbia questo helper

using namespace std;

// Dichiarazioni delle funzioni helper
unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);
unsigned int TextureFromMemory(const aiTexture* aiTex); // <-- NUOVA FUNZIONE HELPER

class Model
{
public:
    // Dati del modello
    vector<Texture> textures_loaded;
    vector<Mesh>    meshes;
    string directory;
    std::map<string, Animation*> m_Animations;

private:
    // <-- MODIFICA 1: Aggiunto un puntatore alla scena di Assimp -->
    // Serve per accedere alle texture incorporate da qualsiasi punto della classe.
    const aiScene* m_scene;

    // Dati per l'animazione
    std::map<string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;

public:
    Model(string const& path)
    {
        loadModel(path);
    }

    ~Model()
    {
        for (auto const& pair : m_Animations)
        {
            delete pair.second;
        }
    }

    void LoadAnimation(const std::string& key, const std::string& path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);
        if (!scene || !scene->mRootNode || scene->mNumAnimations == 0) {
            cout << "ERROR::MODEL::LOAD_ANIMATION: Failed to load animation at path: " << path << endl;
            return;
        }
        m_Animations[key] = new Animation(scene, this);
    }

    void Draw(Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

    auto& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }

private:
    void loadModel(string const& path)
    {
        Assimp::Importer importer;
        // Salva la scena nel membro della classe invece che in una variabile locale
        m_scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        if (!m_scene || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode)
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));
        processNode(m_scene->mRootNode, m_scene);
    }

    void processNode(aiNode* node, const aiScene* scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    void SetVertexBoneDataToDefault(Vertex& vertex)
    {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            vertex.m_BoneIDs[i] = -1;
            vertex.m_Weights[i] = 0.0f;
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            SetVertexBoneDataToDefault(vertex);
            vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);

            if (mesh->HasNormals())
                vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;

                if (mesh->HasTangentsAndBitangents()) {
                    vertex.Tangent = AssimpGLMHelpers::GetGLMVec(mesh->mTangents[i]);
                    vertex.Bitangent = AssimpGLMHelpers::GetGLMVec(mesh->mBitangents[i]);
                }
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        }

        ExtractBoneWeightForVertices(vertices, mesh, scene);

        return Mesh(vertices, indices, textures);
    }

    void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
    {
        for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            int boneID = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
            {
                BoneInfo newBoneInfo;
                newBoneInfo.id = m_BoneCounter;
                newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
                m_BoneInfoMap[boneName] = newBoneInfo;
                boneID = m_BoneCounter;
                m_BoneCounter++;
            }
            else
            {
                boneID = m_BoneInfoMap[boneName].id;
            }
            assert(boneID != -1);
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                assert(vertexId < vertices.size());
                for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
                {
                    if (vertices[vertexId].m_BoneIDs[i] < 0)
                    {
                        vertices[vertexId].m_Weights[i] = weight;
                        vertices[vertexId].m_BoneIDs[i] = boneID;
                        break;
                    }
                }
            }
        }
    }

    // <-- MODIFICA 2: Aggiornata la funzione per caricare le texture -->
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip)
            {
                Texture texture;
                // Controlla se il percorso inizia con '*', indicando una texture incorporata
                if (str.C_Str()[0] == '*') {
                    cout << "Loading embedded texture: " << str.C_Str() << endl;
                    const aiTexture* embeddedTexture = m_scene->GetEmbeddedTexture(str.C_Str());
                    if (embeddedTexture) {
                        texture.id = TextureFromMemory(embeddedTexture);
                    }
                    else {
                        cout << "ERROR::MODEL::COULD_NOT_LOAD_EMBEDDED_TEXTURE: " << str.C_Str() << endl;
                        continue;
                    }
                }
                else {
                    // Altrimenti, carica la texture da un file come prima
                    texture.id = TextureFromFile(str.C_Str(), this->directory);
                }

                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }
        return textures;
    }
};

// <-- MODIFICA 3: Aggiunta la definizione della funzione TextureFromMemory -->
inline unsigned int TextureFromMemory(const aiTexture* aiTex) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = nullptr;

    if (aiTex->mHeight == 0) {
        data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTex->pcData), aiTex->mWidth, &width, &height, &nrComponents, 0);
    }
    else {
        data = reinterpret_cast<unsigned char*>(aiTex->pcData);
        width = aiTex->mWidth;
        height = aiTex->mHeight;
        nrComponents = 4;
    }

    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (aiTex->mHeight == 0) {
            stbi_image_free(data);
        }
    }
    else {
        cout << "Texture from memory failed to load." << endl;
    }

    return textureID;
}

#endif