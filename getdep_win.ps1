wget http://45.63.123.194/sort_dependencies/win/dep.zip -outfile ./easy_profiler.zip

$shell = new-object -com shell.application
$zip = $shell.NameSpace(“.\easy_profiler.zip”)
foreach($item in $zip.items())
{
$shell.Namespace(“.\dependencies\”).copyhere($item)
}