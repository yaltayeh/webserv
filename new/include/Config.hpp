#pragma once

#include <cstddef>

struct Config
{
	const struct Config *parent;

	Config() : parent(NULL) {};
	Config(struct Config *parent) : parent(parent) {};

	virtual ~Config() {};
};
