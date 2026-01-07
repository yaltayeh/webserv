#include "Tree.hpp"

Tree::Tree()
{
}

Tree::Tree(const Tree &other)
{
}

Tree::~Tree()
{
}

Tree Tree::operator=(const Tree &other)
{
	return Tree();
}

const IData &Tree::getData()
{
	// TODO: insert return statement here
}

void Tree::setData(const IData &data)
{
}

const std::string &Tree::getName() const
{
	// TODO: insert return statement here
}

const Tree &Tree::getParent() const
{
	// TODO: insert return statement here
}

void Tree::addChild(Tree &child)
{
}

void Tree::addChild(const IData &data)
{
}

const Tree &Tree::getFirstChild(const std::string &name) const
{
	// TODO: insert return statement here
}

const std::multimap<std::string, Tree> &Tree::getAllChildren() const
{
	// TODO: insert return statement here
}

int Tree::getChildren(const std::string &name, std::vector<Tree> &outStore)
{
	return 0;
}
