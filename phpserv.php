<?php
//foreach ($_GET as $key=>$value)
//{
//      echo $key.": ".$value."<br/>";
//}

header('Content-type: text/json');
$fruits = array (
    "fruits"  => array("a" => "orange", "b" => "banana", "c" => "apple"),
    "numbers" => array(1, 2, 3, 4, 5, 6),
    "holes"   => array("first", 5 => "second", "third"),
    "class"   => "abc"
);
print json_encode($fruits);
?>
