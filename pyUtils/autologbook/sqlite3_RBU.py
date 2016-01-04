#!/usr/bin/python3
# should work in either python2 or 3

import sqlite3

class RBU_table_data:
	"""Information on RBU-cloned table"""
	def __init__(self,tname,curs):
		"""Initialize from table name and cursor to target DB"""
		self.tname = tname

		# load column information
		curs.execute("PRAGMA table_info(%s)"%tname)
		self.cols = [[c[1], c[2].upper().replace("UNIQUE","")] for c in curs.fetchall()]
		self.colnames = [c[0] for c in self.cols]

		# identify primary key (can't handle anything fancy...)
		self.primary = None
		for c in self.cols:
			if "PRIMARY KEY" in c[1]:
				self.primary = c[0]
				c[1] = c[1].replace("PRIMARY KEY", "")
				break
		if self.primary is None:
			self.primary = "rowid"

	def setup_rbu(self, rbu_curs):
		"""Create RBU table"""
		print("Setting up table '%s' in RBU"%self.tname)
		rbucols = ["%s %s"%(c[0],c[1]) if c[1] else c[0] for c in self.cols]
		if self.primary == "rowid":
			rbucols.append("rbu_rowid")
		rbucols.append("rbu_control")
		tcmd = "CREATE TABLE data_%s ("%self.tname + ", ".join(rbucols) + ")"
		print(tcmd)
		rbu_curs.execute(tcmd)		

class RBU_cloner:
	"""Copies sqlite3 commands between active database connecion and Resumable Bulk Update (RBU) copy"""

	def __init__(self, curs, rbu_curs):
		"""Initialize with cursors for primary database and RBU copy"""
		self.curs = curs
		self.rbu_curs = rbu_curs

		# list of tables (with column names) already configured in RBU
		self.tables = {}	
		self.rbu_curs.execute("SELECT name FROM sqlite_master WHERE type = 'table'")
		for nm in self.rbu_curs.fetchall():
			tname = nm[0][5:]
			self.curs.execute("PRAGMA table_info(%s)"%tname)
			self.tables[tname] = RBU_table_data(tname, self.curs)

	def setup_table(self, tname):
		"""Initialize table in RBU"""
		if tname in self.tables:
			return
		self.tables[tname] = RBU_table_data(tname, self.curs)
		self.tables[tname].setup_rbu(self.rbu_curs)

	@staticmethod
	def _insert(curs, tname, valdict, cols = None):
		"""Generate and execute an insert command"""
		if cols is None:
			cols = valdict.keys()
		icmd = "INSERT INTO %s("%tname + ", ".join(cols) + ") VALUES (" + ",".join(["?"]*len(cols)) +")"
		vals = tuple([valdict.get(c,None) for c in cols])
		if curs:
			curs.execute(icmd, vals)
		else:
			print(icmd, vals)

	def insert(self, tname, valdict):
		"""Insert a row into named table, given dictionary of column name/values"""
		self.setup_table(tname)

		self._insert(self.curs, tname, valdict)
		valdict["rbu_control"] = 0
		self._insert(self.rbu_curs, "data_"+tname, valdict)

	def update(self, tname, whereclause, updvals):
		"""Perform an 'update' operation, specified by a WHERE clause and dictionary of update columns"""
		self.setup_table(tname)

		# determine primary key for affected rows
		pkey = self.tables[tname].primary
		self.curs.execute("SELECT %s FROM %s WHERE %s"%(pkey, tname, whereclause))
		rws = [r[0] for r in self.curs.fetchall()]

		# update main DB
		ucmd = "UPDATE " + tname + " SET " + ", ".join([c+" = ?" for c in updvals.keys()]) + " WHERE " + whereclause
		self.curs.execute(ucmd, tuple(updvals.values()))

		# generate RBU commands for each row
		rbucols = self.tables[tname].colnames
		updvals["rbu_control"] = ''.join(['x' if c in updvals else '.' for c in rbucols])
		if pkey == "rowid":
			rbucols.append("rbu_rowid")
			pkey = "rbu_rowid"
		rbucols.append("rbu_control")
		for r in rws:
			updvals[pkey] = r
			self._insert(self.rbu_curs, "data_"+tname, updvals, rbucols)
		
	def delete(self, tname, whereclause):
		"""Perform a 'delete' operation, specified by a WHERE clause"""
		self.setup_table(tname)

		# determine primary key for affected rows
		pkey = self.tables[tname].primary
		self.curs.execute("SELECT %s FROM %s WHERE %s"%(pkey, tname, whereclause))
		rws = [r[0] for r in self.curs.fetchall()]

		# update main DB
		self.curs.execute("DELETE FROM " + tname + " WHERE " + whereclause)

		# generate RBU delete command for each row
		if pkey == "rowid":
			pkey = "rbu_rowid"
		for r in rws:
			self._insert(self.rbu_curs, "data_"+tname, {pkey: r, "rbu_control": 1})

if __name__ == "__main__":
	conn = sqlite3.connect('test.db')
	conn_RBU = sqlite3.connect('test_rbu.db')
	
	R = RBU_cloner(conn.cursor(), conn_RBU.cursor())
	R.insert("foo2", {"a":8, "b":"Hello"})
	R.insert("foo2", {"a":12, "b":"Goodbye"})
	R.update("foo2","a = 8", {"b":9})
	R.delete("foo2","a = 8")

	conn.commit()
	conn_RBU.commit()

