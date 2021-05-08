class ItemCargoCoordinates
{
    int m_row;
    int m_col;
    bool m_flipped

    void ItemCargoCoordinates(int row, int col, bool flipped)
    {
        m_row = row;
        m_col = col;
        m_flipped = flipped;
    }

    int GetRow()
    {
        return m_row;
    }

    int GetCol()
    {
        return m_col;
    }
    
    bool IsFlipped()
    {
        return m_flipped;
    }
}