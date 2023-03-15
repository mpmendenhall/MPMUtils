#! /bin/env python3
## @file JobManager.py job submission manager for cluster computing environments
# Michael P. Mendenhall, LLNL 2020

from JobManager_DB import *
from JobManager_Moab import *
from JobManager_Slurm import *
from JobManager_Local import *
from JobManager_LSF import *
from argparse import ArgumentParser, SUPPRESS
import subprocess

def choose_interface(conn):
    """Identify batch manager interface"""
    if not subprocess.call(["which","sbatch"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL):
        print("Using slurm/sbatch batch system")
        return SlurmInterface(conn)
    if not subprocess.call(["which","msub"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL):
        print("Using moab/msub batch system")
        return MoabInterface(conn)
    if not subprocess.call(["which","jsrun"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL):
        print("Using LSF/bsub batch system")
        return LSFInterface(conn)
    print("No batch system; local job submission.")
    return LocalInterface(conn)

def make_test_jobs(curs, njobs, q, acct, jname = None):
    """Generate test run batch"""
    jcmds = ['echo "Hello world %i!"\nsleep 5\necho "Goodbye!"'%i + ("\nexit 99" if not (i+1)%5 else "") for i in range(njobs)]
    make_upload_shelljobs(curs, "test" if jname is None else jname, q, acct, jcmds, {"walltime": 60, "cores": 1})

def run_commandline():
    global dbfile

    parser = ArgumentParser()

    parser.add_argument("--account",  help="submission billing account")
    parser.add_argument("--queue",    help="submission queue --- set to 'local' for local run")
    parser.add_argument("--db",       help="jobs database", default=dbfile)

    parser.add_argument("--limit",    type=int,   help="concurrent jobs limit")
    parser.add_argument("--trickle",  type=float, help="time delay [s] between nominal run starts")

    parser.add_argument("--name",     help="name for job(s)")
    parser.add_argument("--jobfile",  help="run one-liners in file")
    parser.add_argument("--script",   action="store_true", help="supply script on stdin")
    parser.add_argument("--walltime", type=int, help="wall time for 1-liner jobs in seconds", default = 1800)
    parser.add_argument("--cores",    type=int, default=1, help="cores for 1-liner jobs")

    parser.add_argument("--launch",   action="store_true", help="update and launch")
    parser.add_argument("--cycle",    type=float, help="continuously re-check/launch jobs at specified interval")
    parser.add_argument("--status",   action="store_true", help="update and display status")
    parser.add_argument("--rtimes",   help="display run times stats for named batch")

    #parser.add_argument("--cancel",   action="store_true", help="cancel queued jobs")
    parser.add_argument("--kill",     action="store_true", help="kill running jobs")
    parser.add_argument("--clear",    action="store_true", help="clear completed jobs")
    parser.add_argument("--bundle",   type=int, help="bundle jobs together into group of this walltime")
    parser.add_argument("--test",     type=int, help="run test idle jobs")

    parser.add_argument("--hold",     action="store_true",    help="place all waiting jobs on hold status")
    parser.add_argument("--unhold",   action="store_true",    help="move all held jobs to waiting")

    options = parser.parse_args()

    dbfile = options.db
    conn = connect_JobsDB(dbfile)
    curs = conn.cursor()

    if options.hold:
        curs.execute("UPDATE jobs SET status = -1 WHERE status = 0")
        conn.commit()
        return

    if options.unhold:
        curs.execute("UPDATE jobs SET status = 0 WHERE status = -1")
        conn.commit()
        return

    if options.limit:
        cores_resource_id = get_resource_id(curs, "cores", "number of cores")
        set_resource_limit(curs, cores_resource_id, options.limit)

    display_resource_use(curs)

    if options.rtimes: completed_runstats(curs, options.rtimes)

    jcmds = []
    jname = options.name
    if options.jobfile:
        if not jname: jname = options.jobfile.split("/")[-1]
        jcmds = [l.strip() for l in open(options.jobfile,"r").readlines() if l[0]!='#']
    if options.script:
        print("Input job script on stdin; end with ctrl-D:")
        cmd = sys.stdin.read().strip()
        if cmd: jcmds.append(cmd)
    if jcmds:
        make_upload_shelljobs(conn.cursor(), jname if jname else 'anon', options.queue, options.account,
                              jcmds, {"walltime": options.walltime, "cores": options.cores})
        conn.commit()

    if options.clear: clear_completed(conn)

    if options.test:
        make_test_jobs(curs, options.test, options.queue, options.account, options.name);
        conn.commit()

    # requires batch system interface
    if options.launch or options.status or options.kill or options.cycle or options.bundle:
        BI = choose_interface(conn)
        if options.bundle: rebundle(conn.cursor(), options.bundle, BI.nbundle()); conn.commit()
        if options.launch: BI.update_and_launch(options.trickle)
        if options.status: BI.update_qstatus()
        #if options.cancel: BI.cancel_queued_jobs()
        if options.kill: BI.kill_jobs()
        if options.cycle: BI.cycle_launch(trickle=options.trickle, twait=options.cycle)

    summarize_DB_runstatus(curs)
    conn.close()

"""
rm -r ~/jobs/; ./JobManager.py --test 10 --limit 4 --cycle 5

rm -r ~/jobs/
./JobManager.py --test 300 --queue pbatch --account nuphys --bundle 300
./JobManager.py --cycle 5 --limit 100
"""

if __name__ == "__main__": run_commandline()

