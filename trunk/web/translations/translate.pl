#!/usr/bin/perl
#
# Routino translation replacement Perl script
#
# Part of the Routino routing software.
#
# This file Copyright 2014-2015 Andrew M. Bishop
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

use strict;

# Constants

my @translation_files=(<translation.*.txt>);
my $xml_output_file="../../xml/routino-translations.xml";
my $html_output_dir="../www/routino";

my @html_template_files=(<*.html>);

my %languages=();
my %translations=();


# Sort so that English is first

@translation_files=sort {if($a eq "translation.en.txt"){return -1;} if($b eq "translation.en.txt"){return 1;} return $a <=> $b;} @translation_files;

# Read in the translations

foreach my $translation_file (@translation_files)
  {
   $translation_file =~ m%translation.([^.]+).txt%;

   # Add to list of languages

   my $language=$1;

   if(! defined $languages{$language})
     {
      $languages{$language}=1;

      $translations{$language}={};
      $translations{$language}->{codes}={};
      $translations{$language}->{html}=0;
      $translations{$language}->{xml}=0;
     }

   # Process the file

   open(FILE,"<$translation_file");

   while(<FILE>)
     {
      s%\r*\n%%;

      next if(m%^#%);
      next if(m%^$%);

      # Single line HTML entries

      if(m%\@\@%)
        {
         my($code,$text)=split("\t");

         if(defined $translations{$language}->{codes}->{$code})
           {
            print STDERR "Language: $language DUPLICATED codeword '$code'\n";
           }
         else
           {
            $translations{$language}->{html}++;
            $translations{$language}->{codes}->{$code}={};
            $translations{$language}->{codes}->{$code}->{text}=$text;
            $translations{$language}->{codes}->{$code}->{usedX}=0;
            $translations{$language}->{codes}->{$code}->{usedR}=0;
            $translations{$language}->{codes}->{$code}->{usedV}=0;
           }
        }

      # Multi-line HTML entries

      if(m%(\$\$[^\$]+\$\$)%)
        {
         my($code,$text)=($1,"");

         while(<FILE>)
           {
            last if(m%\$\$%);

            $text=$text.$_;
           }

         $text =~ s%\r*\n$%%;

         if(defined $translations{$language}->{codes}->{$code})
           {
            print STDERR "Language: $language DUPLICATED codeword '$code'\n";
           }
         else
           {
            $translations{$language}->{html}++;
            $translations{$language}->{codes}->{$code}={};
            $translations{$language}->{codes}->{$code}->{text}=$text;
            $translations{$language}->{codes}->{$code}->{usedX}=0;
            $translations{$language}->{codes}->{$code}->{usedR}=0;
            $translations{$language}->{codes}->{$code}->{usedV}=0;
           }
        }

      # Single line XML entries

      if(m%\%\%%)
        {
         my($code,$text)=split("\t");

         if(defined $translations{$language}->{codes}->{$code})
           {
            print STDERR "Language: $language DUPLICATED codeword '$code'\n";
           }
         else
           {
            $translations{$language}->{xml}++;
            $translations{$language}->{codes}->{$code}={};
            $translations{$language}->{codes}->{$code}->{text}=$text;
            $translations{$language}->{codes}->{$code}->{usedX}=0;
            $translations{$language}->{codes}->{$code}->{usedR}=0;
            $translations{$language}->{codes}->{$code}->{usedV}=0;
           }

         my($n_strings_en)=$translations{en}->{codes}->{$code}->{text} =~ s/%s/%s/g;
         my($n_strings)   =$text =~ s/%s/%s/g;

         if($n_strings != $n_strings_en)
           {
            print STDERR "Language: $language WRONG number of '%s' in text '$text' ($translations{en}->{codes}->{$code}->{text})\n";
           }
        }
     }

   close(FILE);
  }


# Sort out the languages

my @languages=();

push(@languages,"en");

foreach my $language (sort (keys %languages))
  {
   push(@languages,$language) if($language ne "en");
  }


# Create the HTML files

foreach my $html_template_file (@html_template_files)
  {
   my $usedtype="";

   $usedtype="R" if($html_template_file =~ m%router%);
   $usedtype="V" if($html_template_file =~ m%visualiser%);

   foreach my $language (@languages)
     {
      next if(!$translations{$language}->{html});

      print "Language: $language File: $html_template_file\n";

      my $language_meta=0;
      my $language_meta_string="";

      open(HTML_IN ,"<$html_template_file");
      open(HTML_OUT,">$html_output_dir/$html_template_file.$language");

      while(<HTML_IN>)
        {
         my $line=$_;

         # Language selection - special handling

         if($line =~ m%\*\*LANGUAGES-META\*\*%)
           {
            $language_meta=1-$language_meta;

            if($language_meta==0)
              {
               foreach my $language2 (@languages)
                 {
                  my $LANGUAGE2=$language2;
                  $LANGUAGE2 =~ tr%a-z%A-Z%;

                  $line=$language_meta_string;

                  if($language eq $language2)
                    {
                     $line =~ s%~~CHECKED~~%checked%g;
                    }
                  else
                    {
                     $line =~ s%~~CHECKED~~%%g;
                    }

                  $line =~ s%~~lang~~%$language2%g;
                  $line =~ s%~~LANG~~%$LANGUAGE2%g;

                  if(!$translations{$language2}->{html})
                    {
                     $line =~ s%<a.+</a>%%;
                    }

                  if(!$translations{$language2}->{xml})
                    {
                     $line =~ s%<input .+>%%;
                    }

                  foreach my $code (keys %{$translations{$language2}->{codes}})
                    {
                     if($line =~ s%$code%$translations{$language2}->{codes}->{$code}->{text}%g)
                       {$translations{$language2}->{codes}->{$code}->{"used$usedtype"} = 1;}
                    }

                  if($line =~ m%((\@\@|\$\$|\*\*|\~\~)[^\@\$*~]+(\@\@|\$\$|\*\*|\~\~))%)
                    {
                     print STDERR "   Unmatched codeword '$1' in line: $line";
                    }

                  # Remove un-needed spaces

                  $line =~ s%[\t ]+% %g;
                  $line =~ s%\n %\n%g;
                  $line =~ s%^ %%g;

                  print HTML_OUT $line;
                 }
              }

            next;
           }

         if($language_meta)
           {
            $language_meta_string.=$line;
            next;
           }

         # Replace with translated phrases

         foreach my $code (keys %{$translations{$language}->{codes}})
           {
            if($line =~ s%\Q$code\E%$translations{$language}->{codes}->{$code}->{text}%g)
              {$translations{$language}->{codes}->{$code}->{"used$usedtype"} = 1;}
           }

         # Replace what is left with English phrases

         foreach my $code (keys %{$translations{$languages[0]}->{codes}})
           {
            $line =~ s%\Q$code\E%$translations{$languages[0]}->{codes}->{$code}->{text}%g;
           }

         if($line =~ m%((\@\@|\$\$|\*\*|\~\~)[^\@\$*~]+(\@\@|\$\$|\*\*|\~\~))%)
           {
            print STDERR "   Unmatched codeword '$1' in line: $line";
           }

         # Remove un-needed spaces

         $line =~ s%[\t ]+% %g;
         $line =~ s%\n %\n%g;
         $line =~ s%^ %%g;

         print HTML_OUT $line;
        }

      close(HTML_IN);
      close(HTML_OUT);
     }
  }


# Create the XML file

open(XML_OUT,">$xml_output_file");

open(XML_IN ,"<translations-head.xml");

while(<XML_IN>)
  {
   print XML_OUT;
  }

close(XML_IN);

foreach my $language (@languages)
  {
   next if(!$translations{$language}->{xml});

   print "Language: $language File: translations.xml\n";

   open(XML_IN ,"<translations-body.xml");

   while(<XML_IN>)
     {
      my $line=$_;

      $line =~ s%~~lang~~%$language%g;

      # Replace with translated phrases

      foreach my $code (keys %{$translations{$language}->{codes}})
        {
         if($line =~ s%$code%$translations{$language}->{codes}->{$code}->{text}%g)
           {$translations{$language}->{codes}->{$code}->{usedX} = 1;}
        }

      # Replace what is left with a note about missing translations

      if($line =~ m%\%\%%)
        {
         foreach my $code (keys %{$translations{$languages[0]}->{codes}})
           {
            $line =~ s%$code%$translations{$languages[0]}->{codes}->{$code}->{text}%g;
           }

         $line =~ s%<%<!-- TRANSLATION REQUIRED: %;
         $line =~ s%>% -->%;

         if($line =~ m%((\%\%|\~\~)[^\%~]+(\%\%|\~\~))%)
           {
            print STDERR "   Unmatched codeword '$1' in line: $line";
           }
        }

      print XML_OUT $line;
     }

   close(XML_IN);
  }

open(XML_IN ,"<translations-tail.xml");

while(<XML_IN>)
  {
   print XML_OUT;
  }

close(XML_IN);

close(XML_OUT);


# Check the languages and usage

my %usedX=();
my %usedR=();
my %usedV=();

foreach my $language (@languages)
  {
   $usedX{$language}=0;
   $usedR{$language}=0;
   $usedV{$language}=0;

   foreach my $code (keys %{$translations{$language}->{codes}})
     {
      $usedX{$language}+=$translations{$language}->{codes}->{$code}->{usedX};
      $usedR{$language}+=$translations{$language}->{codes}->{$code}->{usedR};
      $usedV{$language}+=$translations{$language}->{codes}->{$code}->{usedV};

      if(! $translations{$language}->{codes}->{$code}->{usedX} &&
         ! $translations{$language}->{codes}->{$code}->{usedR} &&
         ! $translations{$language}->{codes}->{$code}->{usedV})
        {
         print STDERR "Language: $language UNUSED codeword: $code\n";
        }
     }
  }


# Print the translation coverage

print "\n";

print "Translation Coverage\n";
print "====================\n";

print "\n";
print "           Number      Percentage Complete\n";
print "Language  XML HTML    XML router visualiser\n";
print "--------  --- ----    --- ------ ----------\n";

foreach my $language (@languages)
  {
   printf("%-6s    %3d  %3d  %4.0f%%  %4.0f%%  %4.0f%%\n",
          $language,
          $translations{$language}->{xml},
          $translations{$language}->{html},
          100.0*$usedX{$language}/$usedX{$languages[0]},
          100.0*$usedR{$language}/$usedR{$languages[0]},
          100.0*$usedV{$language}/$usedV{$languages[0]})
  }
