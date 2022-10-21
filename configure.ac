#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.63)
AC_INIT([greasy],[2.2.3],[support@bsc.es])
AC_CONFIG_SRCDIR([src/greasylog.h])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])
AC_LANG([C++])

AC_SUBST(greasy_engine,[basic])

# Checks for programs.
AC_PROG_CC([gcc cc icc])

AC_CONFIG_MACRO_DIR([m4])
# LT_INIT([dlopen])
# AC_DISABLE_STATIC

# Checks for SLURM

AC_ARG_ENABLE(slurm,
	[  --enable-slurm		enable SLURM (default=yes)],
	[slurm=${enableval}],
	[slurm=yes])

AM_CONDITIONAL(SLURM_LAUNCHER, test x$slurm = xyes)

if test x$slurm = xyes; then
				AC_CHECK_PROG(SLURM_present, srun, [yes], [no])
				AM_CONDITIONAL(SLURM, test x$SLURM_present = xyes)
				if test x$SLURM_present = xyes; then
					AC_DEFINE(SLURM,1, SLURM enabled)
				fi
fi

# Checks for LSF

AC_ARG_ENABLE(lsf,
	[  --enable-lsf 			enable LSF (default=yes)],
	[lsf=${enableval}],
	[lsf=yes])

AM_CONDITIONAL(LSF_LAUNCHER, test x$lsf = xyes)

if test x$lsf = xyes; then
	AC_CHECK_PROG(LSF_present, bjobs, [yes], [no])
	AM_CONDITIONAL(LSF, test x$LSF_present = xyes)
	if test x$LSF_present = xyes; then
 		AC_DEFINE(LSF,1, LSF enabled)
	fi
else
	AM_CONDITIONAL(LSF, false)
	AC_DEFINE(LSF, 0, LSF not enabled)
fi


# Checks for PBS

AC_ARG_ENABLE(pbs,
	[  --enable-pbs				enable PBS (default=yes)],
	[pbs=${enableval}],
	[pbs=yes])

AM_CONDITIONAL(PBS_LAUNCHER, test x$pbs = xyes)

if test x$pbs = xyes; then
                AC_CHECK_PROG(PBS_present, qsub, [yes], [no])
               AM_CONDITIONAL(PBS, test x$PBS_present = xyes)
          if test x$PBS_present = xyes; then
                AC_DEFINE(PBS,1, PBS enabled)
          fi
fi

# Option to compile documentation
AC_ARG_ENABLE(doc,
    [  --enable-doc          enable DOC (default=yes)],
    [doc=${enableval}],
    [doc=yes])

AM_CONDITIONAL(COMPILEDOC, test x$doc = xyes)


# Checks for MPI-ENGINE

AC_ARG_ENABLE(mpi-engine,
  [  --enable-mpi-engine		enable MPI engine (default=no)],
  [mpi_engine=${enableval}],
  [mpi_engine=no])

AM_CONDITIONAL(MPI_ENGINE, test x$mpi_engine = xyes)

if test x$mpi_engine = xyes; then
  AC_DEFINE(MPI_ENGINE, 1, MPI Engine enabled)
  AC_CHECK_PROGS(MPICXX, mpicxx mpic++ CC, $MPICXX)
  CXX=$MPICXX
  AC_MSG_CHECKING([whether the MPI command works]) # be paranoid
  AC_TRY_LINK(
    [#include <mpi.h>],[int argc; char **argv; MPI_Init(&argc,&argv);],
    AC_MSG_RESULT([yes]),
    AC_MSG_ERROR([Cannot compile and link MPI code with $CXX])
    exit;
  )
  AC_PROG_CXX
  AC_SUBST(greasy_engine,[mpi])

  AC_CHECK_PROGS(machine_mpirun,[mnrun srun mpirun mpiexec],[no-mpirun-found])
  if test x$machine_mpirun = xno-mpirun-found; then
    AC_MSG_ERROR([Fatal Error: no MPI executer found])
    exit;
  fi
  if test x$machine_mpirun = xmpirun; then
    AC_PATH_PROGS(machine_mpirun,[mpirun],[no-mpirun-found])
  fi


else
  AC_PROG_CXX
fi

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([limits.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
# AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getcwd gethostname gettimeofday regcomp])

# Checks for Basic engine
AC_CHECK_PROGS(basic_spawn, [srun ssh],[no])
if test x$basic_spawn != xno; then
  if test x$basic_spawn = xsrun; then
    AC_SUBST(remotemethod,["srun --cpu-bind=none"])
  else
    if test x$basic_spawn = xssh; then
      AC_SUBST(remotemethod,[ssh])
    else
      AC_SUBST(remotemethod,[none])
    fi
  fi
fi


# Checks for SLURM-ENGINE

# AC_ARG_ENABLE(slurm-engine,
#   [  --enable-slurm-engine			enable SLURM engine (Not yet implemented)],
#   [slurm_engine=${enableval}],
#   [slurm_engine=no])

# WARNING: implement slurm engine before enabling in Makefile.am
# AM_CONDITIONAL(SLURM_ENGINE, test x$slurm_engine = xyes)
#
# if test x$slurm_engine = xyes; then
#
#   AC_CHECK_PROG(srun_present,[srun],[yes],[no])
#   if test x$srun_present = xno; then
#     AC_MSG_ERROR([Fatal Error: srun command not found])
#     exit;
#   fi

# TODO: implement slurm engine before enabling
#   AC_SUBST(greasy_engine,[slurm])

# fi

# Checks for THREAD-ENGINE

AC_ARG_ENABLE(thread-engine,
  [  --enable-thread-engine	enable THREAD engine. This option will compile a local distribution of TBB by default. If --with-tbb is set it will used the specified installation of TBB.],
  [thread_engine=${enableval}],
  [thread_engine=no])

# check for threads TBB
AM_CONDITIONAL(THREAD_ENGINE, test x$thread_engine = xyes)

# check for local TBB
AC_ARG_WITH([tbb],
   [AS_HELP_STRING([--with-tbb], [Path to the system TBB installation. Otherwise will use the TBB version distributed with GREASY])],
   [tbb=$withval],
   [tbb=no])


AM_CONDITIONAL(TBB_LOCAL, false )
if test x$thread_engine = xyes; then

  AC_DEFINE(THREAD_ENGINE, 1, Thread Engine enabled with TBB)

   if test x$tbb != xno; then
	# use system TBB
	echo "Using system TBB in $tbb"
	AC_SUBST(tbb_include,[$tbb/include])
	AC_SUBST(tbb_libs,[$tbb/libs])
   else
	echo "Using TBB provided by GREASY...will compile TBB sources"
	AC_SUBST(my_tbb_root,[3rdparty/tbb])
	AM_CONDITIONAL(TBB_LOCAL, true )
	AC_SUBST(tbb_libs,[$prefix/lib])
	echo "Uncompressing TBB source..."
	cd src/3rdparty
	tar xvzf tbb40_20111130oss_src.tgz >/dev/null
	ln -s tbb40_20111130oss tbb 2>/dev/null
	cd - > /dev/null
   fi
else
   if test x$tbb != xno; then
	echo "Skipping '--with-tbb' option ... if you want to use TBB you must '--enable-thread-engine'"
   fi
fi

AC_SUBST(greasy_bindir,[$prefix/bin])
AC_SUBST(greasy_etcdir,[$prefix/etc])
AC_SUBST(greasy_libdir,[$prefix/lib])

AC_CONFIG_FILES([Makefile src/Makefile etc/Makefile doc/Makefile etc/greasy.conf example/Makefile example/bsc_greasy.slurm.job example/bsc_greasy.lsf.job example/bsc_greasy.pbs.job bin/Makefile bin/greasy])
AC_OUTPUT

