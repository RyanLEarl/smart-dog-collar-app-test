directories = [
  '',
  'Function',
  'Documentation',
  'Contact'
  
]

var getDirectories = directories;

function createHead() {
  const head = document.getElementById("head");
  var title = document.createElement("Title");
  title.innerHTML = "Smart Dog Collar"
  head.appendChild(title);
}

function createHeader() {
  const header = document.getElementById("header");
  var nav = document.createElement("nav");
  var menu = document.createElement("div")
  nav.setAttribute("id", "nav")
  var name = document.createElement("a");
  name.setAttribute("class", "companyName")
  name.setAttribute("href", "/")
  name.innerHTML = "Smart Dog Collar"
  nav.appendChild(name)
  menu.setAttribute("class", "menu")
  for (i = 0; i<= getDirectories.length-1; i++) {
    var list = document.createElement("li");
    var item = document.createElement("a");
    item.setAttribute("class", "btn");
    if (getDirectories[i] == "") {
      getDirectories[i] = "Home";
      item.setAttribute("href", "/");
    }
    else {
      item.setAttribute("href", "/pages/"+getDirectories[i]);
    }
    item.innerHTML = getDirectories[i];
    list.appendChild(item)
    menu.appendChild(list);
    nav.appendChild(menu);
  }
  header.appendChild(nav);
}