#ifndef _MATRIX_H_
#define _MATRIX_H_

class Matrix
{
	Matrix(const Matrix&);
	Matrix &operator=(const Matrix&);
public:
	Matrix(size_t rows, size_t columns):
		m_rows(rows),
		m_columns(columns),
		m_data(new int[m_rows * m_columns])
	{}

	~Matrix()
	{
		delete[] m_data;
	}

	size_t rows() const {
		return m_rows;
	}

	size_t columns() const {
		return m_columns;
	}

	int &get(int row, int column) {
		return m_data[row * m_columns + column];
	}

	int get(int row, int column) const {
		return m_data[row * m_columns + column];
	}

private:
	size_t m_rows, m_columns;
	int *m_data;
};

#endif /* _MATRIX_H_ */
