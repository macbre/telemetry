<?php

	/*
	 * Telemetria
	 *
	 * Skrypt tworzy firmware dla systemu kompresując podane pliki do pliku binarnego,
	 * ktory jest następnie przesyłany pakietami UDP do hosta, który zapisuje je do strony pamięci EEPROM (256 bajtów).
	 *
	 * (c) Maciej Brencz 2008
	 *
	 */

error_reporting(E_ALL);
 
// IP systemu
$host = '192.168.1.12';

// port
$port = '1500';



// typ pakietu
define('UDP_SEND',   1);	// wyslanie danych do pamieci EEPROM
define('UDP_VERIFY', 2);  // weryfikacja (pobranie) danych z pamięci EEPROM
define('UDP_SEND_OK',3);  // pakiet zwrotny po wgraniu danych do pamieci EEPROM



// pliki do utworzenia firmware'u
$files = array
(
	1 => 'head.htm',
	2 => 'info.htm',
	3 => 'setup.htm',
	4 => 'pwm.htm',
	5 => 'ident.htm',
	6 => 'daq.htm',
	7 => 'daq_no_card.htm',
	8 => 'daq_list.htm',
	10 => 'favicon.png',
	11 => 'loading.gif',
	20 => 'telemetry.css',
	21 => 'css.css',
	22 => 'daq.css',
	30 => 'util.js',
	31 => 'js.js',
	32 => 'pwm.js',
	33 => 'ident.js',
	34 => 'info.js',
	35 => 'daq.js',
	36 => 'daq_list.js',
);


function getmicrotime() { 
    list($usec, $sec) = explode(" ",microtime()); 
    return ((float)$usec + (float)$sec); 
} 

// przesyła dane $data pakietem UDP
//
// @see http://pl.php.net/manual/pl/function.socket-sendto.php#57746
function send_udp($op, $offset, $data) {
	global $host, $port;
	
	// stworz pakiet danych do wyslania
	//
	// bajt 0    - kod operacji
	// bajty 1-4 - offset, pod ktory wpisac dane (little endian)
	// bajt 5    - liczba bajtow do wpisania
	//
	$packet = pack("CVC", $op, $offset, !empty($data) ? strlen($data) : 0) . ( !empty($data) ? $data : '' );

	// utwórz gniazdo UDP
	$socket = @socket_create(AF_INET, SOCK_DGRAM, SOL_UDP);
	
	
	if (!$socket)
		return false;
	
	// czekaj 2 sekundy na reakcje drugiej strony
	//stream_set_timeout($socket, 2);
	
	// wyślij pakiet
	socket_sendto($socket, $packet, strlen($packet), 0, $host, $port);
	
	// czekaj na ramke potwierdzenia
	$data = true;
	
	usleep(40); // 40ms

	$data = socket_read($socket, 1024, PHP_BINARY_READ);
	
	// zamknij gniazdo UDP
	socket_close($socket);
		
	return $data;
}



// wysyla testowe dane do systemu i dokonuje ich weryfikacji
function test() {
	// test
	for ($i=0; $i<4*2; $i++) {
		//send_udp(UDP_SEND, 0x100*$i, 'foo bar'.$i."\x01\x02\x03qwerty");
		send_udp(UDP_SEND, 0x100*$i, $i. str_repeat("qwerty", 35));
		usleep(10000); // 0.01s
	}
	
	// weryfikacja
	for ($i=0; $i<4*2; $i++) {
		send_udp(UDP_VERIFY, 0x100*$i, false);
		usleep(10000); // 0.01s
	}
}


chdir(dirname(__FILE__));

$start = getmicrotime();

echo "\n\tSystem telemetryczny\n\t(c) Maciej Brencz 2008\n-----------------------------------------\n";
echo "Generowanie danych firmware'u...\n\n";

// statystyki
$total_size = 0;
$total_pages = 0;

// strony do zapisania do pamieci
$pages = array();

// rozmiary danych w sektorach
$sizes = array(0 => 0);

// obrobka kolejnych plikow
foreach($files as $id => $file) {

	$src = 'files/'.$file;
	
	echo "\n\t * ".$src.'...';
	
	if (!is_file($src)) {
		die('ERR: "'.$src.'" nie istnieje!');
	}
	
	if (filesize($src) > 5*255) {
		die('ERR: "'.$src.'" ma rozmiar ponad 1275 bajtow');
	}
	
	$fp = fopen($src, 'rb');
	
	$n = 0;
	
	// dziel na strony po 255 bajtow
	while (!feof($fp)) {
			$page  = fread($fp, 255);
			$page .= str_repeat("\x00", 255 - strlen($page));
	
			$pages[$id*5 + ($n++)] = $page;

			$total_pages++;
	}
	
	fclose($fp);
	
	$sizes[$id] = filesize($src);
	$total_size += filesize($src);
	
	echo ' [ok]';
}


// generuj sektor "zero" z dlugoscia plikow
$pages[0] = '';

for ($i=0; $i <= max(array_keys($sizes)); $i++) {
	$pages[0] .= pack('v', isset($sizes[$i]) ? $sizes[$i] : 0);
}

$pages[0] .= str_repeat("\x00", 255 - strlen($pages[0]));


// wyslij kolejne strony
ksort($pages);

echo "\n\nWysylam...";

// dane są gotowe... wyslij je do systemu
foreach($pages as $id => $page) {
	echo "\n\t * #".$id.' ';
		send_udp(UDP_SEND, 0x100*$id, $page);
	echo '[ok]';
}


echo " \n\nWeryfikacja...";

// i zweryfikuj
foreach($pages as $id => $page) {
	$data = send_udp(UDP_VERIFY, 0x100*$id, false);
	
	echo "\n\t * #".$id.' ';
	
	if (substr($data, 6, 255) == $page) {
		echo '[ok]';
	}
	else {
		echo '[err!]';
	}
}

$end = getmicrotime();

echo "\n\n-- Gotowe --";

echo "\n\nWyslano " . round($total_size/1024,2) . 'kB danych na ' . $total_pages . ' stronach w czasie ' . round($end-$start, 2) . "s\n\n";
