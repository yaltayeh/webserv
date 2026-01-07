#pragma once

class IData
{
private:
	
public:
	virtual ~IData() = 0;
};


template <typename T>
class Data: public IData
{
private:
	T value;

public:
	Data();
	virtual ~Data();

	Data(Data &data);
	virtual Data operator=(const Data &data);
	const T &getValue() const;
	void setValue(const T &value);
};


