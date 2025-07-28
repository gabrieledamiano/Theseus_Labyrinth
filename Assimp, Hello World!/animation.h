#ifndef ANIMATION_H
#define ANIMATION_H

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "bone.h"

// Forward declaration per rompere la dipendenza circolare con model.h
class Model;

struct AssimpNodeData
{
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<AssimpNodeData> children;
};

class Animation
{
public:
	Animation(const aiScene* scene, Model* model);
	~Animation();

	Bone* FindBone(const std::string& name);

	float GetTicksPerSecond() { return m_TicksPerSecond; }
	float GetDuration() { return m_Duration; }
	const AssimpNodeData& GetRootNode() { return m_RootNode; }
	const std::map<std::string, BoneInfo>& GetBoneIDMap() { return m_BoneInfoMap; }

private:
	void ReadMissingBones(const aiAnimation* animation, Model& model);
	void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);

	float m_Duration;
	int m_TicksPerSecond;
	std::vector<Bone> m_Bones;
	AssimpNodeData m_RootNode;
	std::map<std::string, BoneInfo> m_BoneInfoMap;
};

#endif