-- sqlite3 database schema for ENSDF card information
-- sqlite3 ENSDF.db < $MPMUTILS/Physics/ENSDF_DB.sql

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

-- card lines, minimally processed; rowid creates unique identifier for records
CREATE TABLE ENSDF_lines (
    card INTEGER,       -- rowid from ENSDF_cards
    mass INTEGER,       -- isotope mass number A
    elem VARCHAR(3),    -- element symbol
    rectp CHAR(3),      -- 3-character record type
    txt CHAR(72)        -- remainder of text
);
CREATE INDEX idx_ENSDF_lines on ENSDF_lines(card);

----------------------
-- "parsed" records --
----------------------

-- comment records
CREATE TABLE comment_records (
    line INTEGER,       -- line to which this comment pertains
    PSYM CHAR(1),       -- particle symbol
    SYM TEXT,           -- symbol(s) being commented on
    CTEXT TEXT          -- text of comment
);
CREATE INDEX idx_comment_records on comment_records(line);

-- continuation data fields for a line
CREATE TABLE record_xdata (
    line INTEGER,       -- rowid from ENSDF_lines this pertains to
    quant TEXT,         -- quantity being defined
    op CHAR(2),         -- operator symbol =, <, >, <=, >=
    val TEXT,           -- value, as text with possible units and uncertainty
    refs TEXT           -- references
);
CREATE INDEX idx_record_xdata on record_xdata(line);

-- Alpha records
CREATE TABLE alpha_records (
    line INTEGER,       -- rowid from ENSDF_lines defining this entry
    tolvl INTEGER,      -- rowid from level_records this decays into
    E REAL,             -- Alpha energy [keV]
    DE REAL,            -- uncertainty on E
    IA REAL,            -- intensity of alpha branch as % of total alpha decay
    DIA REAL,           -- uncertainty on IA
    HF REAL,            -- Hindrance factor of the alpha decay
    DHF REAL,           -- uncertainty on HF
    C CHAR(1),          -- comment flag
    Q CHAR(1)           -- questionable flag
);
CREATE UNIQUE INDEX idx_alpha_records on alpha_records(line);

-- Beta records
CREATE TABLE beta_records (
    line INTEGER,       -- rowid from ENSDF_lines defining this entry
    tolvl INTEGER,      -- rowid from level_records this decays into
    E REAL,             -- Beta endpoint energy [keV], only if measured
    DE REAL,            -- uncertainty on E
    IB REAL,            -- intensity of beta- branch (per NORMALIZATION)
    DIB REAL,           -- uncertainty on IB
    LOGFT REAL,         -- log(ft) for transition
    DFT REAL,           -- uncertainty on LOGFT
    C CHAR(1),          -- comment flag
    UN CHAR(2),         -- Forbidenness
    Q CHAR(1)           -- questionable flag
);
CREATE UNIQUE INDEX idx_beta_records on beta_records(line);

-- Delayed particle records
CREATE TABLE dptcl_records (
    line INTEGER,       -- rowid from ENSDF_lines defining this entry
    tolvl INTEGER,      -- rowid from level_records this decays into
    ptcl VARCHAR(1),    -- symbol for delayed particle
    E REAL,             -- Energy of particle [keV]
    DE REAL,            -- uncertainty on E
    IP REAL,            -- intensity of branch as % of total delayed emissions
    DIP REAL,           -- uncertainty on IP
    EI REAL,            -- energy of "intermediate" level [keV]
    T REAL,             -- width of transition [keV]
    DT REAL,            -- uncertainty on T
    L VARCHAR(9),       -- Angular momentum transfer of emitted particle
    C CHAR(1),          -- comment flag
    COIN CHAR(1),       -- coincidence flag
    Q CHAR(1)           -- questionable flag
);
CREATE UNIQUE INDEX idx_dptcl_records on dptcl_records(line);

-- Electron capture, beta+ records
CREATE TABLE ec_records (
    line INTEGER,       -- rowid from ENSDF_lines defining this entry
    tolvl INTEGER,      -- rowid from level_records this decays into
    E REAL,             -- Electron capture energy [keV], only if measured
    DE REAL,            -- uncertainty on E
    IB REAL,            -- intensity of beta+ branch (per NORMALIZATION)
    DIB REAL,           -- uncertainty on IB
    IE REAL,            -- intensity of electron capture branch (per NORMALIZATION)
    DIE REAL,           -- uncertainty on IE
    LOGFT REAL,         -- log(ft) for ec/beta+ transition
    DFT REAL,           -- uncertainty on LOGFT
    TI REAL,            -- total ec/beta+ transition intensity (per NORMALIZATION)
    DTI REAL,           -- uncertainty on TI
    C CHAR(1),          -- comment flag
    UN CHAR(2),         -- Forbidenness
    Q CHAR(1)           -- questionable flag
);
CREATE UNIQUE INDEX idx_ec_records on ec_records(line);

-- Gamma records
CREATE TABLE gamma_records (
    line INTEGER,       -- rowid from ENSDF_lines defining this entry
    fromlvl INTEGER,    -- rowid from level_records this gamma comes from (NULL if unassigned)
    probto INTEGER,     -- likeliest target rowid from level_records
    E REAL,             -- energy [keV] of gamma
    DE REAL,            -- uncertainty on E
    RI REAL,            -- relative photon intensity
    DRI REAL,           -- uncertainty on RI
    M TEXT,             -- multipolarity
    MR REAL,            -- mixing ratio
    DMR REAL,           -- uncertainty on MR
    CC REAL,            -- total conversion coefficient
    DCC REAL,           -- uncertainty on CC
    TI REAL,            -- relative total transition intensity
    DTI REAL,           -- uncertainty on TI
    C CHAR(1),          -- comment flag
    COIN CHAR(1),       -- coincidence flag
    Q CHAR(1)           -- questionable flag
);
CREATE UNIQUE INDEX idxgamma_records on gamma_records(line,fromlvl);

-- Level records
CREATE TABLE level_records (
    line INTEGER,       -- rowid from ENSDF_lines defining this entry
    E REAL,             -- energy [keV] for decaying level
    DE REAL,            -- uncertainty on E
    J VARCHAR(18),      -- spin and parity
    T REAL,             -- half-life [s]
    DT REAL,            -- uncertainty on T
    L TEXT,             -- angular momentum transfer
    S REAL,             -- spectroscopic strength
    DS REAL,            -- uncertainty on S
    C CHAR(1),          -- comment flag
    MS TEXT,            -- metastable state
    Q CHAR(1)           -- questionable flag
);
CREATE UNIQUE INDEX idxlevel_records on level_records(line,E);

-- Parent records
CREATE TABLE parent_records (
    line INTEGER,       -- rowid from ENSDF_lines defining this entry
    n INTEGER,          -- multiple-parent identifier
    E REAL,             -- energy [keV] for decaying level
    DE REAL,            -- uncertainty on E
    J VARCHAR(18),      -- spin and parity
    T REAL,             -- half-life [s]
    DT REAL,            -- uncertainty on T
    QP REAL,            -- ground-state Q value [keV]
    DQP REAL,           -- uncertainty on QP
    ION INTEGER         -- ionization state for ionized atom decay
);
CREATE UNIQUE INDEX idx_parent_records on parent_records(line);
