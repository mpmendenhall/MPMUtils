-- sqlite3 database schema for storing ENDF6 information
-- sqlite3 $ENDFDB < $MPMUTILS/Physics/RefDataDB/ENDF6_DB_Schema.sql

-- File sections, containing a particular type of information about a particular material
CREATE TABLE ENDF_Sections (
    section_id INTEGER PRIMARY KEY, -- database identifier
    MAT INTEGER,                    -- material identifier
    MF INTEGER,                     -- file type number
    MT INTEGER,                     -- file section number
    A  INTEGER,                     -- material A
    Z  INTEGER,                     -- material Z
    lines TEXT,                     -- text representation of contained info
    pcl BLOB                        -- python3 pickle interpreted object
);
CREATE INDEX idx_ENDF_Sections ON ENDF_Sections(MAT,MF,MT);

-- Index to MF 8 MT 457 decay data entries
CREATE TABLE MF8_MT457_directory (
    section_id INTEGER PRIMARY KEY, -- section in database
    A INTEGER,                      -- nucleus A
    Z INTEGER,                      -- nucleus Z
    LIS INTEGER,                    -- excitation state number
    LISO INTEGER,                   -- isomeric state number
    FOREIGN KEY(section_id) REFERENCES ENDF_Sections(section_id) ON DELETE CASCADE
);
CREATE UNIQUE INDEX idx_MF8_MT457_directory ON MF8_MT457_directory(A,Z,LIS,LISO);
