#include <map>
#include <vector>
#include <string>
#include "Data.hpp"

#pragma once

class Tree
{
private:
	IData *data;
	std::multimap<std::string, Tree> children;
	Tree *parent;
	std::string name;

public:
	Tree();
	Tree(const Tree &other);
	~Tree();
	Tree operator=(const Tree &other);
	const IData &getData();
	void setData(const IData &data);

	const std::string &getName() const;
	const Tree &getParent() const;
	void addChild(Tree &child);
	void addChild(const IData &data);
	const Tree &getFirstChild(const std::string &name) const;
	int getChildren(const std::string &name, std::vector<Tree> &outStore);
	const std::multimap<std::string, Tree> &getAllChildren() const;
};
