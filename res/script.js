"use strict"; // plsssssssxxs
let direction = annotationsJSON.dir === undefined ? "UD" : annotationsJSON.dir;
let levelSeparation = 200;
let nodeSpacing = 130;


if(direction === "UD" || direction === "DU") {
    levelSeparation = 120;
    nodeSpacing = 200;
}

let nodes = [];

for (let el of annotationsJSON.code_annotations) {
    nodes.push({
        id: el.id,
        label: el.id + "\n" + el.title,
        color: {
            border: '#733e0e',
            background: '#ff9635',
            highlight: {
                border: '#73190e',
                background: '#da3521'
            },
            hover: {
                border: '#73310e',
                background: '#da4d21'
            }
        },
    });
}

for (let el of annotationsJSON.text_annotations) {
    nodes.push({ id: el.id, label: el.id + "\n" + el.title });
}


let edges = [];

for (let el of annotationsJSON.text_annotations) {
    for (let ref of el.references) {
        edges.push({
            from: el.id,
            to: ref,
        });
    }
}

nodes.forEach(node => {
    node.sortkey = "0";
});

nodes.forEach(node => {
    edges.forEach(edge => {
        if (edge.to == node.id) {
            var parent = nodes.find(e => e.id == edge.from);
            node.sortkey = parent.sortkey + "." + node.id;
        }
    })
});

nodes = nodes.sort((a, b) => {
    if (a.sortkey > b.sortkey) return 1;
    if (a.sortkey < b.sortkey) return -1;
    return 0;
});

let container = document.querySelector("#network");

let data = {
    nodes: nodes,
    edges: edges
};

let options = {
    layout: {
        randomSeed: undefined,
        improvedLayout: true,
        clusterThreshold: 150,
        hierarchical: {
            enabled: true,
            levelSeparation: levelSeparation, // 120 vert 170 hor
            nodeSpacing: nodeSpacing,     // 200 vert 100 hor
            treeSpacing: 300,
            blockShifting: true,
            edgeMinimization: true,
            parentCentralization: true,
            direction: direction,        // UD, DU, LR, RL
            sortMethod: 'directed',  // hubsize, directed
            shakeTowards: 'leaves'  // roots, leaves
        }
    },
    edges: {
        arrows: {
            to: true
        },
        shadow: {
            enabled: true
        }
    },
    nodes: {
        shape: "box",
        widthConstraint: true,
        color: {
            border: '#116E00',
            background: '#4CC835',
            highlight: {
                border: '#009c33',
                background: '#06be37'
            },
            hover: {
                border: '#0fb217',
                background: '#3ada42'
            }
        },
        shadow: {
            enabled: true
        }
    },
    physics: false,
    interaction: {
        hover: true,
        //selectable: false
    }

    /*
    physics: {
        hierarchicalRepulsion: {
            avoidOverlap: 1,
            springConstant: 0.001
        },
    },
    */
};

let network = new vis.Network(container, data, options);

let viewer = document.querySelector("#viewer");

function onRefClick(ref) {
    selectNode(ref);
}

function onNodeClick(event) {
    let nodes = event.nodes;
    if (nodes.length === 0) {
        return;
    }
    if (nodes.length > 1) {
        return;
    }
    network.unselectAll();
    selectNode(nodes[0]);

}

function selectNode(nodeId) {
    let isCode = false;
    let annotation = annotationsJSON.text_annotations
        .find((annotation) => nodeId === annotation.id);
    if (annotation === undefined) {
        annotation = annotationsJSON.code_annotations
            .find((annotation) => nodeId === annotation.id);
        isCode = true;
    }
    viewer.style.display = "flex";
    viewer.querySelector("h4").textContent = annotation.id;
    viewer.querySelector("h1").textContent = annotation.title;
    viewer.querySelector(".file")?.remove();
    let content = viewer.querySelector(".content");
    content.innerHTML = "";
    let split = annotation.content.split("\n");
    for(let line of split) {
        if(!isCode) {
            let refs = annotation.references;
            for(let ref of refs) {
                line = line.replace("$" + ref, 
                    `<a href="#${ref}" onclick="onRefClick('${ref}')">$${ref}</a>`);
            }

        }
        let pos = 0;
        for(let char of line) {
            if(char != " ") {
                break;
            }
            pos++;
        }
        for(let i = 0; i < pos; i++) {
            content.innerHTML += "&#x00A0";
        }
        content.innerHTML += line + "<br/>";

    }
    if(isCode) {
        content.style.color = "white";
        content.style.backgroundColor = "rgb(38, 38, 38)";
        content.style.overflowX = "scroll";
        content.style.whiteSpace = "nowrap";
        let file = document.createElement("p");
        file.classList.add("file");
        file.textContent = annotation.file + ":" + annotation.line;
        content.before(file);
    }
    else {
        content.style.color = "black";
        content.style.backgroundColor = "white";
        content.style.overflowX = "hidden";
        content.style.whiteSpace = "normal";
    }
}

network.on("click", onNodeClick);

document.querySelector(".close").onclick = () => {
    viewer.style.display = "none";
}
