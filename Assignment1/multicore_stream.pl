#!/usr/bin/env perl
#
# Multicore STREAM Copy benchmark, using the UNIX shell to launch
# multiple instances of a single-threaded benchmark.

$| = 1;

sub check_requisites
{
  $g_src = "mem_bench.c";
  if(!(-f $g_src)) {
    die qq`
I do not see the source file '$g_src'
Please run this script in a directory that contains the source file.
`;
  }

  $tmp = "mcs-temp";
  if (-d $tmp) {
    # all set
  } elsif (-e $tmp) {
    die qq`
I need to create a directory '$tmp' but there is already something else
with that name!
`;
  } else {
    system('mkdir', $tmp);
    if (-d $tmp) {
      # okay
    } else {
      die qq`
I tried to create a directory '$tmp', but failed!
`;
    }
  }
  $fp = "$tmp/test.txt";
  if (-e $fp) {
    unlink ($fp);
  }
  if (-e $fp) {
    die qq`
I cannot delete files in the directory '$tmp'. This is needed
for the test to work, so I am giving up.
`;
  }
  system('touch', $fp);
  if (!(-f $fp)) {
    die qq`
I cannot create files in the directory '$tmp'. This is needed
for the test to work, so I am giving up.
`;
  }
  unlink($fp);
  if (-e $fp) {
    die qq`
I cannot delete files in the directory '$tmp'. This is needed
for the test to work, so I am giving up.
`;
  }
} # End of check.requisites

# Try to determine the number of threads available on this machine. We use
# two methods, one works on Mac OS X and the other on (most) Linux.
sub count_threads
{
  my($nt, $gc, $pci);

  $nt = 0; # Default
  $gc = "/usr/bin/getconf";
  if (-x $gc) {
    $nt = `$gc _NPROCESSORS_ONLN` + 0;
    if ($nt == 0) {
      # Maybe we're on FreeBSD?
      $nt = `$gc NPROCESSORS_ONLN` + 0;
    }
  } else {
    $pci = "/proc/cpuinfo";
    if (-f $pci) {
      $nt = `grep '^processor' $pci | wc -l` + 0;
    }
  }
  if ($nt <= 0) {
    # Return a 'safe' answer
    $nt = 4;
  }
  return $nt;
} # End of num.threads;

sub one_run
{
  my($nt, $memsize) = @_;
  my($i, $fp, $cmd, $waiting, $IN, $l, $bw);

  # Remove the temp files that will be created by this set of tests.
  for($i=0; $i<$nt; $i++) {
    $fp = "$tmp/output-$i.txt";
    unlink($fp);
  }
  # Run a lot of tests in parallel (note '&' at end of command, so
  # we are running them all in the background
  for($i=0; $i<$nt; $i++) {
    $fp = "$tmp/output-$i.txt";
    system("./mb $memsize > $fp &");
  }
  # Wait until all of them have written to their output file(s)
  $waiting = 1;
  while($waiting) {
    $waiting = 0;
    for($i=0; $i<$nt; $i++) {
      $fp = "$tmp/output-$i.txt";
      if (-s $fp < 10) {
        $waiting = 1;
        $i = $nt;
      }
    }
    sleep(1);
  }
  # Now all jobs are finished, collect and add their output
  $bw = 0;
  for($i=0; $i<$nt; $i++) {
    $fp = "$tmp/output-$i.txt";
    open($IN, $fp);
    $l = <$IN>; chomp $l;
    close $IN;
    if ($l =~ m/memsize +\d+ +([-+.e0-9]+) bytes/) {
      $bw += $1;
    } else {
die qq`
Could not parse output:
  $l
`;
    }
  }
  return $bw;
} # End of one.run

&check_requisites();
$num_threads = &count_threads();
print "This machine supports $num_threads threads.\n";
$num_to_test = 2 * $num_threads;

$maxmem = 64 * 1024 * 1024;
print qq`
We will test up to $num_to_test simultaneous copies of a STREAM memory-copy
benchmark, with buffer sizes up to $maxmem bytes.

The bandwidth is a function of two independent variables: the number of
simultaneous tasks, and the amount of data that each task is repeatedly
copying. For each combination of these, two measurements of bandwidth
are printed out: total bandwidth (achieved by all tasks combined), and
bandwidth per task. Units are:

  tasks: unitless integer
  memory block size: bytes
  total bandwidth: bytes/second
  bandwidth per task: bytes/second (or "bytes per second per task")

This will take some time, and the processor load will steadily increase.
If the computer's fans are making noise, that's probably why.

`;

if (-x "mb") {
  unlink("mb");
}
system("gcc -O1 $g_src -o mb");
if (!(-x "mb")) {
  die qq`
Source file $g_src did not compile.
`;
}

print sprintf("%5s, %8s, %9s, %9s\n",
                            "tasks", "memsize", "bandwidth", "BW/task");
for($i=1; $i<=$num_to_test; $i*=2) {
  for($memsize = 1024; $memsize <= $maxmem; $memsize *= 4) {
    $bw = &one_run($i, $memsize);
    print sprintf("%3d, %8d, %9.4g, %9.4g\n", $i, $memsize, $bw, $bw/$i);
  }
  print "\n";
}
