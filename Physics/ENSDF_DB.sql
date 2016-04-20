-- sqlite3 database schema for ENSDF card information
-- sqlite3 ENSDF.db < ENSDF_DB.sql

-- cards in dataset, with rowid used elsewhere for card identifier; interpreted from Identification record
CREATE TABLE ENSDF_cards (
    mass INTEGER,       -- isotope mass number A
    elem VARCHAR(3),    -- element symbol
    DSID TEXT,          -- dataset ID string
    DSREF VARCHAR(26),  -- publication references
    PUB VARCHAR(9),     -- publication information
    EDATE INTEGER       -- YYYYMM entry date into ENSDF system
);
CREATE UNIQUE INDEX idx_ENSDF_cards ON ENSDF_cards(mass,elem,DSID,EDATE);

-- card lines, minimally processed
CREATE TABLE ENSDF_lines (
    card INTEGER,       -- rowid from ENSDF_cards
    n INTEGER,          -- line number
    rectp CHAR(3),      -- 3-character record type
    text CHAR(72)       -- remainder of text
);
CREATE UNIQUE INDEX idx_ENSDF_lines on ENSDF_lines(card,n);
