<?php 

require_once 'WhatsAppAPI/Twilio/autoload.php'; 
 
use Twilio\Rest\Client; 

if(isset($_GET["sid"]) && isset($_GET["a_token"]) && isset($_GET["body"]) && isset($_GET["from"]) && isset($_GET["to"])){
	$sid    = $_GET["sid"]; 
    $token  = $_GET["a_token"];  
    $twilio = new Client($sid, $token);
	
	$to = $_GET['to'];
	$from = $_GET['from'];
	$body = $_GET['body'];
	
	$message = $twilio->messages 
                  ->create("whatsapp:+".$to, 
                           array( 
                               "from" => "whatsapp:+".$from,       
                               "body" => $body
                           ) 
                  );
    echo 'Message Send...';				  
	
}else{
	echo "There is no request yet!";
}

?>