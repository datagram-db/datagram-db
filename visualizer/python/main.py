__author__ = "Giacomo Bergami"
__copyright__ = "Copyright 2023"
__credits__ = ["Giacomo Bergami"]
__license__ = "GPL"
__version__ = "3.0"
__maintainer__ = "Giacomo Bergami"
__email__ = "bergamigiacomo@gmail.com"
__status__ = "Production"

# import os
import json
import os.path
import sys


import numpy as np
import pandas as pd
import uvicorn
from starlette.responses import HTMLResponse, Response, FileResponse, PlainTextResponse
from fastapi.responses import ORJSONResponse

import parsers.NestedTables
from parsers.GSMExt import GsmSpecification, gsm_object, to_vis_nodes, to_vis_network_phi, phi, to_vis_network_node
from parsers.ReadGSMExt import serialize_gsm_model, deserialize_gsm_model, deserialize_gsm_file
from parsers.SemiringProvenance import ProvenanceObject, provenance_object

from fastapi import Request, FastAPI
from fastapi.middleware.cors import CORSMiddleware
import matplotlib.pyplot as plt
import base64
from io import BytesIO
import seaborn as sns
import matplotlib.pyplot as plt


from watchfiles import watch

app = FastAPI()

origins = [
    "http://localhost",
    "http://localhost:9999",
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

N_result = dict()
E_result = dict()
N_input = dict()
E_input = dict()
N_removed = dict()
N_inserted = dict()
def html_base64_image(fig):
    tmpfile = BytesIO()
    fig.savefig(tmpfile, format='png')
    encoded = base64.b64encode(tmpfile.getvalue()).decode('utf-8')
    return '<img src=\'data:image/png;base64,{}\'>'.format(encoded)



@app.get("/morphisms/{folder}", response_class=HTMLResponse)
async def morphisms(folder):
    return parsers.NestedTables.generate_morphism_html(os.path.join(os.getcwd(), "data", folder), folder)


def load_nodes_any(folder, N, file):
    if folder not in N:
        N[folder] = dict()
        for obj in deserialize_gsm_file(os.path.join(os.getcwd(), "data", folder, file)):
            N[folder][obj.id] = obj
    return N


def load_edges_any(folder, N, E, file):
    N = load_nodes_any(folder, N, file)
    if folder not in E:
        E[folder] = to_vis_network_phi(N[folder].values())
    return (N, E)


def load_result_nodes(folder):
    global N_result
    N_result = load_nodes_any(folder, N_result, "result.json")
    # if folder not in N:
    #     N[folder] = dict()
    #     for obj in deserialize_gsm_file(os.path.join(os.getcwd(),"data",folder,"result.json")):
    #         N[folder][obj.id] = obj


def load_input_nodes(folder):
    global N_input
    global N_removed
    global N_inserted
    with open(os.path.join(os.getcwd(), "data", folder, "removed.json"), "r") as f:
        N_removed[folder] = set(json.load(f))
    with open(os.path.join(os.getcwd(), "data", folder, "inserted.json"), "r") as f:
        N_inserted[folder] = set(json.load(f))
    N_input = load_nodes_any(folder, N_input, "input.json")


def load_result_edges(folder):
    global N_result
    global E_result
    (N_result, E_result) = load_edges_any(folder, N_result, E_result, "result.json")


def load_input_edges(folder):
    global N_input
    global E_input
    (N_input, E_input) = load_edges_any(folder, N_input, E_input, "input.json")


@app.get("/result/nodes/{folder}")
async def nodes(folder):
    load_input_nodes(folder)
    load_result_nodes(folder)
    global N_result
    return to_vis_nodes(N_result[folder].values(), None, N_inserted[folder])


@app.get("/input/nodes/{folder}")
async def inodes(folder):
    load_input_nodes(folder)
    global N_input
    global N_removed
    global N_inserted
    return to_vis_nodes(N_input[folder].values(), N_removed[folder], N_inserted[folder])


@app.get("/result/node/{folder}/{id}")
async def node(folder, id):
    load_input_nodes(folder)
    load_result_nodes(folder)
    global N_result
    global N_inserted
    return to_vis_network_node(N_result[folder][int(id)], None, N_inserted[folder])


from fastapi.responses import JSONResponse


@app.get("/javascript/{file}")
def javascript(file):
    return FileResponse(file)


class CSSResponse(Response):
    media_type = "text/css"


@app.get("/css/{file}")
def javascript(file, response_class=CSSResponse):
    return FileResponse(file)


@app.get("/input/node/{folder}/{id}")
async def inode(folder, id):
    load_input_nodes(folder)
    global N_input
    return to_vis_network_node(N_input[folder][int(id)])


@app.get("/result/edges/{folder}")
async def edges(folder):
    load_result_edges(folder)
    global E_result
    return E_result[folder]


@app.get("/input/edges/{folder}")
async def iedges(folder):
    load_input_edges(folder)
    global E_input
    return E_input[folder]


@app.get("/cypher/{folder}", response_class=PlainTextResponse)
async def cypher(folder):
    from fast_graph.neo4j_conn import ext_neo4j
    return ext_neo4j(folder)

@app.get("/result/graph/{folder}", response_class=HTMLResponse)
async def graph(folder):
    with open("test.html", "r") as f:
        return f.read().replace("§", folder).replace('£', 'result')


@app.get("/input/graph/{folder}", response_class=HTMLResponse)
async def graph(folder):
    with open("test.html", "r") as f:
        content = f.read().replace("§", folder).replace('£', 'input')
        pos = content.find("<div id=\"title\">")
        result = content[:pos] + parsers.NestedTables.generate_morphism_html(os.path.join(os.getcwd(), "data"),
                                                                             folder) + content[pos:]
        return result  # f.read().replace("§", folder).replace('£','input')


def create_button(html, text):
    f"<a href=\"#\" onClick=\"MyWindow=window.open('{html}','MyWindow','width=600,height=300'); return false;\">{text}</a>"

def dropdown(i):
    return f"""    <li><div class="btn-group">
      <button type="button" class="btn btn-primary dropdown-toggle" data-toggle="dropdown">
      Graph #{i} <span class="caret"></span></button>
      <ul class="dropdown-menu" role="menu">
        <li><a href="#" onClick="MyWindow=window.open('/input/graph/{i}','MyWindow','width=600,height=300'); return false;">Input</a></li>
        <li><a href="#" onClick="MyWindow=window.open('/result/graph/{i}','MyWindow','width=600,height=300'); return false;">Result</a></li>
      </ul>
    </div></li>"""

def generate_lists():
    l = [y for x in os.walk("./data") for y in x[1]]
    graphs = "<ul>" + (" ".join(map(dropdown, map(int, sorted(l, key=lambda x: int(x)))))) + "</ul>"
    return graphs

def index_page():
    lists = generate_lists()
    return f"""<html lang="en">
<head>
  <title>Bootstrap Example</title>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/css/bootstrap.min.css">
  <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.7.1/jquery.min.js"></script>
  <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/js/bootstrap.min.js"></script>
</head>
<body>

<div class="container">
  <div class="jumbotron">
    <h1>Query Results</h1>      
    <p>Visualising the query results</p>
  </div>
  <p>{lists}</p>        
</div>

</body>
</html>"""

@app.get("/", response_class=HTMLResponse)
async def lists():
    return index_page()


# @app.post("/renderConfusionMatrix")
# async def get_body(request: Request):
#     """
#     :param request: json request, for example, given a file result.json
#     {
#     "similarity_matrix": [[1,0,0],[0,1,0],[0,0,1]],
#     "sentences": ["a","b","c"]
#     }
#
#     :return: Returns the HTML rendering of this confusion matrix. If you fire the
#     request with the following command:
#
#     curl -X POST -H "Content-Type: application/json" -d @result.json http://127.0.0.1:9999/renderConfusionMatrix > test.html
#
#     you will then obtain the plot in an HTML rendering, quite simplistically.
#     """
#     payload = await request.json()
#     nd_a = np.array(payload["similarity_matrix"])
#     df = pd.DataFrame(nd_a, index=list(payload["sentences"]), columns=list(payload["sentences"]))
#     hm = sns.heatmap(data=df,
#                      annot=True)
#     fig = hm.get_figure()
#     html_content = "<html><body>"+html_base64_image(fig)+"</body></html>"
#     return HTMLResponse(content=html_content, status_code=200)


if __name__ == '__main__':
    if len(sys.argv) > 1:
        path = sys.argv[1]
        if os.path.exists(path):
            os.chdir(path)
    print(sorted([y for x in os.walk("./data") for y in x[1]], key=lambda x:int(x)))
    uvicorn.run("main:app", port=9999, workers=1,
                reload=True,
                reload_includes=["*.json", "*.ncsv"],
                reload_dirs=["./data", "./data/0"])

# # create app
# # app = dash.Dash()
# if __name__ == '__main__':
#     f = deserialize_gsm_file("data/output/result.json")
#     Model = [gsm_object(id=0,ell=["label"],xi=["value"],properties={"key":"value"},phi=[phi(0,"content",1,0)]),
#              gsm_object(id=1,ell=["etichetta"],xi=["valore"],properties={"proprietà":"valore"})]
#     s = to_vis_nodes(Model)
#     p = to_vis_network_phi(Model)
#     print(serialize_gsm_model(Model))
#     print('serialized:', json.dumps(s))
#     print('serialized:', json.dumps(p))
#
#     # serialized: {'boxes': [{'maxclass': 'newobj'}, {'maxclass': 'message'}, {'maxclass': 'int'}]}
#
#     # d2 = deserialize_gsm_model(s)
#     # print('deserialized:', d2)
#     #parsers.NestedTables.generate_morphism_html(sys.argv[1])
