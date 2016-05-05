-- sqlite3 database schema for NIST isotope mass/abundance data
-- sqlite3 Isotopes.db < $MPMUTILS/Physics/RefDataDB/Isotopes_DB.sql

-- Elements: symbols, names, average atomic weight
CREATE TABLE Elements (
    Z INTEGER,          -- atomic number Z
    symbol TEXT,        -- element symbol
    name TEXT,          -- element name
    mass REAL,          -- relative atomic mass
    dmass REAL          -- uncertainty on mass
);
CREATE UNIQUE INDEX idx_Elements_Z ON Elements(Z);
CREATE UNIQUE INDEX idx_Elements_symb ON Elements(symbol);


-- Isotopes
CREATE TABLE Isotopes (
    Z INTEGER,          -- atomic number Z
    A INTEGER,          -- mass number A
    symb TEXT,          -- element/isotope symbol
    mass REAL,          -- relative atomic mass
    dmass REAL,         -- uncertainty on mass
    frac REAL,          -- isotopic fraction
    dfrac REAL          -- uncertainty on fraction
);
CREATE UNIQUE INDEX idx_Isotopes ON Isotopes(Z,A);
