<?php

if($_SERVER['REQUEST_URI'] === '/'){
        header('Location: http://'.$_SERVER['HTTP_HOST'].'/index');
        exit;
}
?>
