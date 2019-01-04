-- sqlite3 database schema for storing ENDF6 information
-- sqlite3 ENDF.db < $MPMUTILS/Physics/RefDataDB/ENDF6_DB_Schema.sql

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
CREATE UNIQUE INDEX idx_ENDF_Sections ON ENDF_Sections(MAT,MF,MT);
