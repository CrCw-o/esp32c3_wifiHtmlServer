// note R"KEYWORD( html page code )KEYWORD"; 
// again I hate strings, so char is it and this method let's us write naturally

const char PAGE_MAIN[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
	<script>
		function getPickerColor(senderid){
			var colorSelected = document.getElementById("html5colorpicker").value;
			return colorSelected;
		}
		
		function setLED(ledIndexNumber,rgb) {
			var xhttp = new XMLHttpRequest();
			
			xhttp.onreadystatechange = function() {
					if (this.readyState == 4 && this.status == 200) { 		
            			// ignore  response
					}
				}     		
			xhttp.open("POST", "UPDATE_LED?COLOR="+ rgb.substring(1) + "&VALUE="+ledIndexNumber , true);
	  		xhttp.send();
		}
		
		function AddOnClickToTable(){
			
			document.querySelectorAll("td").forEach((td) => {
				td.onclick = function () {var col = getPickerColor(this.id); this.style.backgroundColor = col;setLED(this.id,col);} ;
			});
		}

	</script>
	<style>
		div {background-color: rgb(255, 255, 255);color: rgb(0, 0, 0);padding: 20px;		}
		table, th, td {	  border:1px solid black; border-collapse: collapse;	}
		td {width: 40px;}
		tr {height: 40px;}
	</style>
<body onload="AddOnClickToTable();">
	<div>
		<table id="tabelle">
			<tr><td id="t00">.</td><td id="t01">.</td><td id="t02">.</td><td id="t03">.</td><td id="t04">.</td></tr>
			<tr><td id="t05">.</td><td id="t06">.</td><td id="t07">.</td><td id="t08">.</td><td id="t09">.</td></tr>
			<tr><td id="t10">.</td><td id="t11">.</td><td id="t12">.</td><td id="t13">.</td><td id="t14">.</td></tr>
			<tr><td id="t15">.</td><td id="t16">.</td><td id="t17">.</td><td id="t18">.</td><td id="t19">.</td></tr>
			<tr><td id="t20">.</td><td id="t21">.</td><td id="t22">.</td><td id="t23">.</td><td id="t24">.</td></tr>    	
		</table>
	</div> 

	<div id="html5DIV">
		<h3>Farbe</h3>
		<input type="color" id="html5colorpicker" value="#ff0000" style="width:85%;">
	</div>
</body>
</html>
)=====";