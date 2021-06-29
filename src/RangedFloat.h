#ifndef RANGEDFLOAT_H
#define RANGEDFLOAT_H

class RangedFloat
{
private:
	float min;
	float max;
	float data;
	
public:
	RangedFloat(float max)
	{
		this->min = 0.0f;
		this->max = max;
		data = 0.0f;
	}

	RangedFloat(float min, float max)
	{
		this->min = min;
		this->max = max;
		data = 0.0f;
	}

    inline int Int() { return (int)data; }
    inline float Float() { return data; }
	
	inline RangedFloat& operator=(const RangedFloat& rhs)
	{
		data = rhs.data;
		return *this;
	}
	
	inline RangedFloat& operator=(const float& rhs)
	{
		data = rhs;
		return *this;
	}
	
	inline RangedFloat& operator+=(const RangedFloat& rhs)
	{
		data += rhs.data;
		if (data > max)
			data -= max;
		return *this;
	}
	
	inline RangedFloat& operator+=(const float& rhs)
	{
		data += rhs;
		if (data > max)
			data -= max;
		return *this;
	}
	
	inline RangedFloat& operator-=(const RangedFloat& rhs)
	{
		data -= rhs.data;
		if (data < min)
			data += max;
		return *this;
	}
	
	inline RangedFloat& operator-=(const float& rhs)
	{
		data -= rhs;
		if (data < min)
			data += max;
		return *this;
	}
	
	inline RangedFloat& operator++()
	{
		data++;
		return *this;
	}
	
	inline RangedFloat operator++(int)
	{
		RangedFloat rf(*this);
		operator++();
		return rf;
	}
	
	inline RangedFloat& operator--()
	{
		data--;
		return *this;
	}
	
	inline RangedFloat operator--(int)
	{
		RangedFloat rf(*this);
		operator--();
		return rf;
	}

	inline operator float() const { return (float)data; }
    inline operator int() const { return (int)data; }
};

inline float operator+(RangedFloat lhs, const float& rhs) { return lhs += rhs; }
inline float operator+(const float& lhs, RangedFloat rhs) { return rhs += lhs; }
inline float operator-(RangedFloat lhs, const float& rhs) { return lhs -= rhs; }
inline float operator-(const float& lhs, RangedFloat rhs) { return rhs -= lhs; }

#endif