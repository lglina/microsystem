#!/usr/bin/perl

use File::Basename;

$filename = $ARGV[0];
$maxsize = $ARGV[1];
@fileparts = fileparse($filename, qr/\.[^.]*/);
$basename = @fileparts[0];
$varname = $basename;
$varname =~ s/-/_/g;
$extension = @fileparts[2];
$filesize = -s $filename;
if(defined($maxsize))
{
    $filesize = $maxsize
}

open(ANS, $filename);
binmode(ANS);

print "// ${basename}${extension}\n";
print "const int ${varname}_size( ${filesize} )\;\n";
print "const unsigned char ${varname}[${varname}_size] =\n";
print "{\n";
print "    ";

$bytecount = 0;
$linecount = 0;
$firstchar = 1;
$c = getc(ANS);
while(defined($c) and ($bytecount < $filesize))
{
    if( $c eq "\x1a" )
    {
        #last;
    }

    if( $linecount == 0 )
    {
        if( $firstchar )
        {
            $firstchar = 0;
        }
        else
        {
            print ",\n    ";
        }
    }
    else
    {
        print ", ";
    }

    print "0x" . unpack("H*", $c);
    $c = getc(ANS);
    $bytecount++;
    $linecount++;
    
    if($linecount == 10)
    {
        $linecount = 0;
    }
}

print "\n};\n";
