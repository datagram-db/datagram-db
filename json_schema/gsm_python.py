# generated by datamodel-codegen:
#   filename:  test.schema.json
#   timestamp: 2023-09-18T18:37:05+00:00
#   command: "datamodel-codegen --input test.schema.json --output model.py"

from __future__ import annotations

from typing import Any, Dict, List, Optional

from pydantic import BaseModel, Extra, Field


class PhiItem(BaseModel):
    containment: str = Field(
        description='the containment relation associating the parent object to the included child',
    )
    content: int = Field(description='object id providing a content to the object')
    score: Optional[float] = Field(
        '1.0',
        description='confidence value of the parent object containing the content object',
    )


class GsmSpecification(BaseModel):
    class Config:
        extra = Extra.forbid

    id: int = Field(description='A unique identifier for an GSM object')
    scores: List[float] = Field(
        description='list of confidence scores associated to the object',
        min_items=1,
    )
    ell: List[str] = Field(
        description='list of labels/types associated to the object',
        min_items=0,
    )
    xi: List[str] = Field(
        description='list of string-based values associated to the object',
        min_items=0,
    )
    properties: Dict[str, str] = Field(
        description='key-value representation associated to each object',
    )
    phi: List[PhiItem] = Field(
        description='associating a content label to some content',
        min_items=0
    )
