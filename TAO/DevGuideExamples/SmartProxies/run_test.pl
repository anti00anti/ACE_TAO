# $Id$

eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
    & eval 'exec perl -S $0 $argv:q'
    if 0;

use Env (ACE_ROOT);
use lib "$ACE_ROOT/bin";
use PerlACE::Run_Test;


$M_ior = "Messenger.ior";
$L_ior = "Logger.ior";

unlink $M_ior;
unlink $L_ior;

# start MessengerServer
$S = new PerlACE::Process("MessengerServer");
$S->Spawn();
if (PerlACE::waitforfile_timed ($M_ior, 5) == -1) {
    print STDERR "ERROR: cannot find file $M_ior\n";
    $S->Kill(); 
    exit 1;
}

# start LoggerServer
$L = new PerlACE::Process("LoggerServer");
$L->Spawn();
if (PerlACE::waitforfile_timed ($L_ior, 5) == -1) {
    print STDERR "ERROR: cannot find file $L_ior\n";
    $L->Kill(); 
    $S->Kill();
    exit 1;
}
  
# start MessengerClient
$C = new PerlACE::Process("MessengerClient");
if ($C->SpawnWaitKill(20) != 0) {
     $S->Kill();
     $L->Kill();
     exit (1);
}

# clean-up 
$S->Kill();
$L->Kill();
unlink $M_ior;
unlink $L_ior;

exit 0;



