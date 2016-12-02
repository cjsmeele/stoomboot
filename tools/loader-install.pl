#!/usr/bin/env perl

# Copyright (c) 2014, 2015 Chris Smeele
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

use 5.10.1;

use strict;
use warnings;

no warnings "experimental::smartmatch";

use Getopt::Long;

my $STAGE1_DAP_OFFSET          = 0x05; # Offset to the start of the DAP struct.
my $STAGE1_LOADER_FS_ID_OFFSET = $STAGE1_DAP_OFFSET + 0x10 + 1; # DAP + boot device id.

my $o_mbr = 0;
my $o_fat;
my $o_img;
my $o_stage1_mbr;
my $o_stage1_fat;
my $o_stage2;
my $o_stage2_offset;
my $o_stage2_blocks;
my $o_loader_fs_id = 0;

GetOptions(
	"mbr!"            => \$o_mbr,
	"fat32=i"         => \$o_fat,
	"img=s"           => \$o_img,
	"stage1-mbr=s"    => \$o_stage1_mbr,
	"stage1-fat32=i"  => \$o_stage1_fat,
	"stage2=s"        => \$o_stage2,
	"stage2-lba=i"    => \$o_stage2_offset,
	"stage2-blocks=i" => \$o_stage2_blocks,
	"loader-fs-id=s"  => \$o_loader_fs_id,
);

$o_loader_fs_id = hex $o_loader_fs_id;

die "$0: no disk image specified\n"    if not defined $o_img;
die "$0: no stage2 offset specified\n" if not defined $o_stage2_offset;
die "$0: must specify either stage2 or stage2-blocks\n" unless defined $o_stage2 xor defined $o_stage2_blocks;
die "$0: writing bootsectors to FAT32 partitions is not yet supported\n" if defined $o_fat;

sub slurp {
	my $fn = shift;
	open my $fh, '<', $fn or die "$0: could not open file '$fn' for reading: $!\n";
	binmode $fh;
	local $/ = undef;
	my $content = <$fh>;
	close $fh;
	return $content;
}

open my $img, '+<', $o_img or die "$0: could not open image file: $!\n";
binmode $img;

my @stage2;
if (defined $o_stage2) {
	my $stage2 = slurp $o_stage2;
	   @stage2 = unpack 'C*', $stage2;
	$o_stage2_blocks = int (@stage2 / 512 + (@stage2 % 512 ? 1 : 0));
}

die "$0: stage2 too large (max 64K)\n" if length($o_stage2_blocks) > 64*1024;

if ($o_mbr) {
	die "$0: no mbr stage1 image specified\n" if not defined $o_stage1_mbr;
	my $stage1 = slurp $o_stage1_mbr;
	my @stage1 = unpack 'C*', $stage1;

	die "$0: stage1 bootsector should be exactly 512 bytes long\n"
		if @stage1 != 512;

	die "$0: bytes 437..510 in mbr stage1 should be set to NUL (is your stage1 code too long?)\n"
		if grep { $_ } @stage1[436..509];

	die "$0: stage1 bootsector does not contain magic 0x55 0xaa string\n"
		unless [@stage1[510, 511]] ~~ [0x55, 0xaa];

	die "$0: first instruction in stage1 bootsector should be a far jump (0xea)\n"
		if $stage1[0] != 0xea;

	# Write bootsector.
	print $img pack 'C*', @stage1[0..435];

	# Write stage2 LBA.
	seek $img, $STAGE1_DAP_OFFSET + 0x08, 0;
	print $img pack 'Q', $o_stage2_offset;

	# Write stage2 block count into the DAP.
	seek $img, $STAGE1_DAP_OFFSET + 0x02, 0;
	print $img pack 'C', $o_stage2_blocks;

	# Write loader filesystem id.
	seek $img, $STAGE1_LOADER_FS_ID_OFFSET, 0;
	print $img pack 'Q', $o_loader_fs_id;

	# Write bootsector magic.
	seek $img, 510, 0;
	print $img pack 'C*', 0x55, 0xaa;
}

if (defined $o_stage2) {
	seek $img, $o_stage2_offset*512, 0;
	print $img pack 'C*', @stage2;
}

close $img;
