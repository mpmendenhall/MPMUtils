autologbook lab logging utility suite
Michael P. Mendenhall (2016)

a collection of Python scripts for generating and managing sqlite3 database log files of lab "slow control" readings


Logger:
	- reads in datapoints from devices
	- writes to logbook DB file, possible remote copies
	- checks for "actionable" conditions (out-of-range values, etc.)
	- manages rollover/archiving of logbook DB files
	- needs to be fully fault tolerant

Locator:
	- interface for locating and collecting archived data for analysis tasks

Plotter:
	- locates applicable files
	- in-memory data caches, incrementally updated (minimize thrashing of DB file)
	- receives plot requests

Display:
	- web interface or local "live" display

dynamic mode: webpage is generated for each request
cached mode: webpage is cached, re-generated on request
static: non-interactive webpage is pre-generated at regular intervals
