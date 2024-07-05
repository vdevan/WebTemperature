char Style[] PROGMEM  = R"rawliteral(
<style>
body {
background-color: #b4e0b4;
}
h4 {
margin: 2px;
}
h1 {
color:blue;
text-align:center;
}
.container {
margin: auto;
width: 90%;
min-width: 540px;
background-color: #b5eafe;
padding: 0px 10px;
}
@media(min-width:1200px) {
.container {
margin: auto;
width: 45%;
}
}
@media(min-width:768px) and (max-width:1200px) {
.container {
margin: auto;
width: 55%;
}
}
.q {
float: right;
width: 64px;
text-align: right;
}
.l {
background: url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==') no-repeat left center;
background-size: 1em;
}
.scnnet {
background-color: beige;
padding: 5px;
margin-bottom: 5px;
}
.strnet {
background-color: bisque;
padding: 5px;
}
.inf {
padding: 2px;
}
textarea {
white-space: pre;
}
table {
width: 95%;
margin: auto;
border - collapse: collapse;
}

p {
margin-block: 0.8em;
}

td {
padding: 2px;
}
td.stbtn {
border: 1px solid #808080;
padding: 2px 10px;
width: 90%;
}
.ti {
width:95%;
}
.btn {
border-radius: 4px;
border: 0;
color: red;
cursor: pointer;
display: inline-block;
margin: 2px;
padding: 5px;
position: relative;
background-color: beige;
box-shadow: 0px 6px 5px -4px rgba(0,0,0,0.52);
border: 0;
}
.btn:hover {
background: #b4e0b4;
}
.btn:active, #btn:focus {
background: #b5eafe;
}
.btn:disabled {
background-color: rgba(100,100,100,0.1);
cursor: not-allowed;
}
.txt {
padding: 5px;
width: 140px;
}
.attn {
color: red;
font-weight: 600;
}
div[contenteditable] {
outline: none;
height:100px; 
column-width:12em;
background-color:azure;
padding:10px;
border:inset;
color:darkolivegreen;
}

input[type="file"] {
  display:none;
}
</style>
)rawliteral";
