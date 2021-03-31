#!/usr/bin/php
<?php
function generate_macro($num) : string {
        $returner = "#define str".$num."cmp_macro(ptr, ";
        for($x = 0; $x < $num; $x++){
                $returner .= "c".$x;
                if($x != $num-1){ $returner .= ", "; }
        }
        $returner .= ") ";
        for($x = 0; $x < $num; $x++){
                $returner .= "*(ptr+".$x.") == c".$x;
                if($x != $num-1){ $returner .= " && "; }
        }
        return $returner;
}

function generate_static_inline_fn(&$generated_macro, $num) : string {
        $generated_macro .= "\nstatic bool str".$num."cmp(const char* ptr, const char* cmp)".
                                "{\n\t\treturn str".$num."cmp_macro(ptr, ";
        for($x = 0; $x < $num; $x++){
                $generated_macro .= " *(cmp+".$x.")";
                if($x != $num-1){ $generated_macro .= ", "; }
        }
        $generated_macro .= ");\n}\n";
        return $generated_macro;
}

function handle_generation($argc, $argv) : void {
        $out_filename = $argv[$argc-1];
        $gen_macro = "";
        for($x = 0; $x < $argc-2; $x++){
                $macro = generate_macro($argv[$x+1])."\n";
                $gen_macro .= generate_static_inline_fn($macro, $argv[$x+1]);
        }
        file_put_contents($out_filename, $gen_macro);
}

handle_generation($argc, $argv);
